static void perf_event_interrupt(struct pt_regs *regs)
{
	int i;
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);
	struct perf_event *event;
	unsigned long val;
	int found = 0;
	int nmi;
	if (cpuhw->n_limited)
		freeze_limited_counters(cpuhw, mfspr(SPRN_PMC5),
					mfspr(SPRN_PMC6));
	perf_read_regs(regs);
	nmi = perf_intr_is_nmi(regs);
	if (nmi)
		nmi_enter();
	else
		irq_enter();
	for (i = 0; i < cpuhw->n_events; ++i) {
		event = cpuhw->event[i];
		if (!event->hw.idx || is_limited_pmc(event->hw.idx))
			continue;
		val = read_pmc(event->hw.idx);
		if ((int)val < 0) {
			found = 1;
			record_and_restart(event, val, regs, nmi);
		}
	}
	if (!found) {
		for (i = 0; i < ppmu->n_counter; ++i) {
			if (is_limited_pmc(i + 1))
				continue;
			val = read_pmc(i + 1);
			if ((int)val < 0)
				write_pmc(i + 1, 0);
		}
	}
	write_mmcr0(cpuhw, cpuhw->mmcr[0]);
	if (nmi)
		nmi_exit();
	else
		irq_exit();
}