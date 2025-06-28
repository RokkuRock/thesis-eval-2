static int try_to_unmap_cluster(unsigned long cursor, unsigned int *mapcount,
		struct vm_area_struct *vma, struct page *check_page)
{
	struct mm_struct *mm = vma->vm_mm;
	pmd_t *pmd;
	pte_t *pte;
	pte_t pteval;
	spinlock_t *ptl;
	struct page *page;
	unsigned long address;
	unsigned long mmun_start;	 
	unsigned long mmun_end;		 
	unsigned long end;
	int ret = SWAP_AGAIN;
	int locked_vma = 0;
	address = (vma->vm_start + cursor) & CLUSTER_MASK;
	end = address + CLUSTER_SIZE;
	if (address < vma->vm_start)
		address = vma->vm_start;
	if (end > vma->vm_end)
		end = vma->vm_end;
	pmd = mm_find_pmd(mm, address);
	if (!pmd)
		return ret;
	mmun_start = address;
	mmun_end   = end;
	mmu_notifier_invalidate_range_start(mm, mmun_start, mmun_end);
	if (down_read_trylock(&vma->vm_mm->mmap_sem)) {
		locked_vma = (vma->vm_flags & VM_LOCKED);
		if (!locked_vma)
			up_read(&vma->vm_mm->mmap_sem);  
	}
	pte = pte_offset_map_lock(mm, pmd, address, &ptl);
	update_hiwater_rss(mm);
	for (; address < end; pte++, address += PAGE_SIZE) {
		if (!pte_present(*pte))
			continue;
		page = vm_normal_page(vma, address, *pte);
		BUG_ON(!page || PageAnon(page));
		if (locked_vma) {
			mlock_vma_page(page);    
			if (page == check_page)
				ret = SWAP_MLOCK;
			continue;	 
		}
		if (ptep_clear_flush_young_notify(vma, address, pte))
			continue;
		flush_cache_page(vma, address, pte_pfn(*pte));
		pteval = ptep_clear_flush(vma, address, pte);
		if (page->index != linear_page_index(vma, address)) {
			pte_t ptfile = pgoff_to_pte(page->index);
			if (pte_soft_dirty(pteval))
				pte_file_mksoft_dirty(ptfile);
			set_pte_at(mm, address, pte, ptfile);
		}
		if (pte_dirty(pteval))
			set_page_dirty(page);
		page_remove_rmap(page);
		page_cache_release(page);
		dec_mm_counter(mm, MM_FILEPAGES);
		(*mapcount)--;
	}
	pte_unmap_unlock(pte - 1, ptl);
	mmu_notifier_invalidate_range_end(mm, mmun_start, mmun_end);
	if (locked_vma)
		up_read(&vma->vm_mm->mmap_sem);
	return ret;
}