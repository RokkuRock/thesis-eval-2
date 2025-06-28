void native_tss_update_io_bitmap(void)
{
	struct tss_struct *tss = this_cpu_ptr(&cpu_tss_rw);
	struct thread_struct *t = &current->thread;
	u16 *base = &tss->x86_tss.io_bitmap_base;
	if (!test_thread_flag(TIF_IO_BITMAP)) {
		tss_invalidate_io_bitmap(tss);
		return;
	}
	if (IS_ENABLED(CONFIG_X86_IOPL_IOPERM) && t->iopl_emul == 3) {
		*base = IO_BITMAP_OFFSET_VALID_ALL;
	} else {
		struct io_bitmap *iobm = t->io_bitmap;
		if (tss->io_bitmap.prev_sequence != iobm->sequence)
			tss_copy_io_bitmap(tss, iobm);
		*base = IO_BITMAP_OFFSET_VALID_MAP;
	}
	refresh_tss_limit();
}