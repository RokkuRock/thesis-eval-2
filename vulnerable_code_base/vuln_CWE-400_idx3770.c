static irqreturn_t armv7pmu_handle_irq(int irq_num, void *dev)
{
	unsigned long pmnc;
	struct perf_sample_data data;
	struct cpu_hw_events *cpuc;
	struct pt_regs *regs;
	int idx;
	pmnc = armv7_pmnc_getreset_flags();
	if (!armv7_pmnc_has_overflowed(pmnc))
		return IRQ_NONE;
	regs = get_irq_regs();
	perf_sample_data_init(&data, 0);
	cpuc = &__get_cpu_var(cpu_hw_events);
	for (idx = 0; idx <= armpmu->num_events; ++idx) {
		struct perf_event *event = cpuc->events[idx];
		struct hw_perf_event *hwc;
		if (!test_bit(idx, cpuc->active_mask))
			continue;
		if (!armv7_pmnc_counter_has_overflowed(pmnc, idx))
			continue;
		hwc = &event->hw;
		armpmu_event_update(event, hwc, idx, 1);
		data.period = event->hw.last_period;
		if (!armpmu_event_set_period(event, hwc, idx))
			continue;
		if (perf_event_overflow(event, 0, &data, regs))
			armpmu->disable(hwc, idx);
	}
	irq_work_run();
	return IRQ_HANDLED;
}