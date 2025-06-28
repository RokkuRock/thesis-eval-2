mm_zfree(struct mm_master *mm, void *address)
{
	mm_free(mm, address);
}