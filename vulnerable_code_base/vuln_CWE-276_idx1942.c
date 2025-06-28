static inline void tss_invalidate_io_bitmap(struct tss_struct *tss)
{
	tss->x86_tss.io_bitmap_base = IO_BITMAP_OFFSET_INVALID;
}