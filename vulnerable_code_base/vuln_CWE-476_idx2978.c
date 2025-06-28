void migrate_page_copy(struct page *newpage, struct page *page)
{
	int cpupid;
	if (PageHuge(page) || PageTransHuge(page))
		copy_huge_page(newpage, page);
	else
		copy_highpage(newpage, page);
	if (PageError(page))
		SetPageError(newpage);
	if (PageReferenced(page))
		SetPageReferenced(newpage);
	if (PageUptodate(page))
		SetPageUptodate(newpage);
	if (TestClearPageActive(page)) {
		VM_BUG_ON_PAGE(PageUnevictable(page), page);
		SetPageActive(newpage);
	} else if (TestClearPageUnevictable(page))
		SetPageUnevictable(newpage);
	if (PageChecked(page))
		SetPageChecked(newpage);
	if (PageMappedToDisk(page))
		SetPageMappedToDisk(newpage);
	if (PageDirty(page)) {
		clear_page_dirty_for_io(page);
		if (PageSwapBacked(page))
			SetPageDirty(newpage);
		else
			__set_page_dirty_nobuffers(newpage);
 	}
	if (page_is_young(page))
		set_page_young(newpage);
	if (page_is_idle(page))
		set_page_idle(newpage);
	cpupid = page_cpupid_xchg_last(page, -1);
	page_cpupid_xchg_last(newpage, cpupid);
	ksm_migrate_page(newpage, page);
	if (PageSwapCache(page))
		ClearPageSwapCache(page);
	ClearPagePrivate(page);
	set_page_private(page, 0);
	if (PageWriteback(newpage))
		end_page_writeback(newpage);
}