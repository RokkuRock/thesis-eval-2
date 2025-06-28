static void watchdog_overflow_callback(struct perf_event *event, int nmi,
		 struct perf_sample_data *data,
		 struct pt_regs *regs)
{
	event->hw.interrupts = 0;
	if (__this_cpu_read(watchdog_nmi_touch) == true) {
		__this_cpu_write(watchdog_nmi_touch, false);
		return;
	}
	if (is_hardlockup()) {
		int this_cpu = smp_processor_id();
		if (__this_cpu_read(hard_watchdog_warn) == true)
			return;
		if (hardlockup_panic)
			panic("Watchdog detected hard LOCKUP on cpu %d", this_cpu);
		else
			WARN(1, "Watchdog detected hard LOCKUP on cpu %d", this_cpu);
		__this_cpu_write(hard_watchdog_warn, true);
		return;
	}
	__this_cpu_write(hard_watchdog_warn, false);
	return;
}