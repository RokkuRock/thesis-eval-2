static __always_inline ssize_t __mcopy_atomic_hugetlb(struct mm_struct *dst_mm,
					      struct vm_area_struct *dst_vma,
					      unsigned long dst_start,
					      unsigned long src_start,
					      unsigned long len,
					      bool zeropage)
{
	int vm_alloc_shared = dst_vma->vm_flags & VM_SHARED;
	int vm_shared = dst_vma->vm_flags & VM_SHARED;
	ssize_t err;
	pte_t *dst_pte;
	unsigned long src_addr, dst_addr;
	long copied;
	struct page *page;
	struct hstate *h;
	unsigned long vma_hpagesize;
	pgoff_t idx;
	u32 hash;
	struct address_space *mapping;
	if (zeropage) {
		up_read(&dst_mm->mmap_sem);
		return -EINVAL;
	}
	src_addr = src_start;
	dst_addr = dst_start;
	copied = 0;
	page = NULL;
	vma_hpagesize = vma_kernel_pagesize(dst_vma);
	err = -EINVAL;
	if (dst_start & (vma_hpagesize - 1) || len & (vma_hpagesize - 1))
		goto out_unlock;
retry:
	if (!dst_vma) {
		err = -ENOENT;
		dst_vma = find_vma(dst_mm, dst_start);
		if (!dst_vma || !is_vm_hugetlb_page(dst_vma))
			goto out_unlock;
		if (!dst_vma->vm_userfaultfd_ctx.ctx)
			goto out_unlock;
		if (dst_start < dst_vma->vm_start ||
		    dst_start + len > dst_vma->vm_end)
			goto out_unlock;
		err = -EINVAL;
		if (vma_hpagesize != vma_kernel_pagesize(dst_vma))
			goto out_unlock;
		vm_shared = dst_vma->vm_flags & VM_SHARED;
	}
	if (WARN_ON(dst_addr & (vma_hpagesize - 1) ||
		    (len - copied) & (vma_hpagesize - 1)))
		goto out_unlock;
	err = -ENOMEM;
	if (!vm_shared) {
		if (unlikely(anon_vma_prepare(dst_vma)))
			goto out_unlock;
	}
	h = hstate_vma(dst_vma);
	while (src_addr < src_start + len) {
		pte_t dst_pteval;
		BUG_ON(dst_addr >= dst_start + len);
		VM_BUG_ON(dst_addr & ~huge_page_mask(h));
		idx = linear_page_index(dst_vma, dst_addr);
		mapping = dst_vma->vm_file->f_mapping;
		hash = hugetlb_fault_mutex_hash(h, dst_mm, dst_vma, mapping,
								idx, dst_addr);
		mutex_lock(&hugetlb_fault_mutex_table[hash]);
		err = -ENOMEM;
		dst_pte = huge_pte_alloc(dst_mm, dst_addr, huge_page_size(h));
		if (!dst_pte) {
			mutex_unlock(&hugetlb_fault_mutex_table[hash]);
			goto out_unlock;
		}
		err = -EEXIST;
		dst_pteval = huge_ptep_get(dst_pte);
		if (!huge_pte_none(dst_pteval)) {
			mutex_unlock(&hugetlb_fault_mutex_table[hash]);
			goto out_unlock;
		}
		err = hugetlb_mcopy_atomic_pte(dst_mm, dst_pte, dst_vma,
						dst_addr, src_addr, &page);
		mutex_unlock(&hugetlb_fault_mutex_table[hash]);
		vm_alloc_shared = vm_shared;
		cond_resched();
		if (unlikely(err == -ENOENT)) {
			up_read(&dst_mm->mmap_sem);
			BUG_ON(!page);
			err = copy_huge_page_from_user(page,
						(const void __user *)src_addr,
						pages_per_huge_page(h), true);
			if (unlikely(err)) {
				err = -EFAULT;
				goto out;
			}
			down_read(&dst_mm->mmap_sem);
			dst_vma = NULL;
			goto retry;
		} else
			BUG_ON(page);
		if (!err) {
			dst_addr += vma_hpagesize;
			src_addr += vma_hpagesize;
			copied += vma_hpagesize;
			if (fatal_signal_pending(current))
				err = -EINTR;
		}
		if (err)
			break;
	}
out_unlock:
	up_read(&dst_mm->mmap_sem);
out:
	if (page) {
		if (vm_alloc_shared)
			SetPagePrivate(page);
		else
			ClearPagePrivate(page);
		put_page(page);
	}
	BUG_ON(copied < 0);
	BUG_ON(err > 0);
	BUG_ON(!copied && !err);
	return copied ? copied : err;
}