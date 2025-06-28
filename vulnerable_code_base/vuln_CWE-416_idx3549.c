static inline void get_page(struct page *page)
{
	page = compound_head(page);
	VM_BUG_ON_PAGE(page_ref_count(page) <= 0, page);
	page_ref_inc(page);
}