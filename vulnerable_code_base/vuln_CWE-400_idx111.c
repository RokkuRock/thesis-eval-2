static void record_and_restart(struct perf_event *event, unsigned long val,
			       struct pt_regs *regs, int nmi)
{
	u64 period = event->hw.sample_period;
	s64 prev, delta, left;
	int record = 0;
	if (event->hw.state & PERF_HES_STOPPED) {
		write_pmc(event->hw.idx, 0);
		return;
	}
	prev = local64_read(&event->hw.prev_count);
	delta = (val - prev) & 0xfffffffful;
	local64_add(delta, &event->count);
	val = 0;
	left = local64_read(&event->hw.period_left) - delta;
	if (period) {
		if (left <= 0) {
			left += period;
			if (left <= 0)
				left = period;
			record = 1;
			event->hw.last_period = event->hw.sample_period;
		}
		if (left < 0x80000000LL)
			val = 0x80000000LL - left;
	}
	write_pmc(event->hw.idx, val);
	local64_set(&event->hw.prev_count, val);
	local64_set(&event->hw.period_left, left);
	perf_event_update_userpage(event);
	if (record) {
		struct perf_sample_data data;
		perf_sample_data_init(&data, 0);
		data.period = event->hw.last_period;
		if (perf_event_overflow(event, nmi, &data, regs))
			fsl_emb_pmu_stop(event, 0);
	}
}