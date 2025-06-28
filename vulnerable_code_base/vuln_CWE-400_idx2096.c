void ptrace_triggered(struct perf_event *bp, int nmi,
		      struct perf_sample_data *data, struct pt_regs *regs)
{
	struct perf_event_attr attr;
	attr = bp->attr;
	attr.disabled = true;
	modify_user_hw_breakpoint(bp, &attr);
}