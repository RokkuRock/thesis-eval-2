static int unmap_ref_private(struct mm_struct *mm, struct vm_area_struct *vma,
				struct page *page, unsigned long address)
{
	struct hstate *h = hstate_vma(vma);
	struct vm_area_struct *iter_vma;
	struct address_space *mapping;
	struct prio_tree_iter iter;
	pgoff_t pgoff;
	address = address & huge_page_mask(h);
	pgoff = vma_hugecache_offset(h, vma, address);
	mapping = (struct address_space *)page_private(page);
	mutex_lock(&mapping->i_mmap_mutex);
	vma_prio_tree_foreach(iter_vma, &iter, &mapping->i_mmap, pgoff, pgoff) {
		if (iter_vma == vma)
			continue;
		if (!is_vma_resv_set(iter_vma, HPAGE_RESV_OWNER))
			__unmap_hugepage_range(iter_vma,
				address, address + huge_page_size(h),
				page);
	}
	mutex_unlock(&mapping->i_mmap_mutex);
	return 1;
}