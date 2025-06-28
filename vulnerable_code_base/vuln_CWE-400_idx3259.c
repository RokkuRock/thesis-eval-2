static void __intel_pmu_pebs_event(struct perf_event *event,
				   struct pt_regs *iregs, void *__pebs)
{
	struct pebs_record_core *pebs = __pebs;
	struct perf_sample_data data;
	struct pt_regs regs;
	if (!intel_pmu_save_and_restart(event))
		return;
	perf_sample_data_init(&data, 0);
	data.period = event->hw.last_period;
	regs = *iregs;
	regs.ip = pebs->ip;
	regs.bp = pebs->bp;
	regs.sp = pebs->sp;
	if (event->attr.precise_ip > 1 && intel_pmu_pebs_fixup_ip(&regs))
		regs.flags |= PERF_EFLAGS_EXACT;
	else
		regs.flags &= ~PERF_EFLAGS_EXACT;
	if (perf_event_overflow(event, 1, &data, &regs))
		x86_pmu_stop(event, 0);
}