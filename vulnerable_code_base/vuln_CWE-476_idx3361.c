int migrate_page_move_mapping(struct address_space *mapping,
		struct page *newpage, struct page *page,
		struct buffer_head *head, enum migrate_mode mode,
		int extra_count)
{
	int expected_count = 1 + extra_count;
	void **pslot;
	if (!mapping) {
		if (page_count(page) != expected_count)
			return -EAGAIN;
		set_page_memcg(newpage, page_memcg(page));
		newpage->index = page->index;
		newpage->mapping = page->mapping;
		if (PageSwapBacked(page))
			SetPageSwapBacked(newpage);
		return MIGRATEPAGE_SUCCESS;
	}
	spin_lock_irq(&mapping->tree_lock);
	pslot = radix_tree_lookup_slot(&mapping->page_tree,
 					page_index(page));
	expected_count += 1 + page_has_private(page);
	if (page_count(page) != expected_count ||
		radix_tree_deref_slot_protected(pslot, &mapping->tree_lock) != page) {
		spin_unlock_irq(&mapping->tree_lock);
		return -EAGAIN;
	}
	if (!page_freeze_refs(page, expected_count)) {
		spin_unlock_irq(&mapping->tree_lock);
		return -EAGAIN;
	}
	if (mode == MIGRATE_ASYNC && head &&
			!buffer_migrate_lock_buffers(head, mode)) {
		page_unfreeze_refs(page, expected_count);
		spin_unlock_irq(&mapping->tree_lock);
		return -EAGAIN;
	}
	set_page_memcg(newpage, page_memcg(page));
	newpage->index = page->index;
	newpage->mapping = page->mapping;
	if (PageSwapBacked(page))
		SetPageSwapBacked(newpage);
	get_page(newpage);	 
	if (PageSwapCache(page)) {
		SetPageSwapCache(newpage);
		set_page_private(newpage, page_private(page));
	}
	radix_tree_replace_slot(pslot, newpage);
	page_unfreeze_refs(page, expected_count - 1);
	__dec_zone_page_state(page, NR_FILE_PAGES);
	__inc_zone_page_state(newpage, NR_FILE_PAGES);
	if (!PageSwapCache(page) && PageSwapBacked(page)) {
		__dec_zone_page_state(page, NR_SHMEM);
		__inc_zone_page_state(newpage, NR_SHMEM);
	}
	spin_unlock_irq(&mapping->tree_lock);
	return MIGRATEPAGE_SUCCESS;
}