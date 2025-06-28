static void ptrace_triggered(struct perf_event *bp, int nmi,
			     struct perf_sample_data *data,
			     struct pt_regs *regs)
{
	int i;
	struct thread_struct *thread = &(current->thread);
	for (i = 0; i < HBP_NUM; i++) {
		if (thread->ptrace_bps[i] == bp)
			break;
	}
	thread->debugreg6 |= (DR_TRAP0 << i);
}