static void flush_tmregs_to_thread(struct task_struct *tsk)
{
	if (tsk != current)
		return;
	if (MSR_TM_SUSPENDED(mfmsr())) {
		tm_reclaim_current(TM_CAUSE_SIGNAL);
	} else {
		tm_enable();
		tm_save_sprs(&(tsk->thread));
	}
}