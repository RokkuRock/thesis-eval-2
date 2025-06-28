static inline void switch_to_bitmap(unsigned long tifp)
{
	if (tifp & _TIF_IO_BITMAP)
		tss_invalidate_io_bitmap(this_cpu_ptr(&cpu_tss_rw));
}