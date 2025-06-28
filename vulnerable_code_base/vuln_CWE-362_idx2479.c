static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
			     struct task_struct *tsk)
{
	unsigned cpu = smp_processor_id();
	if (likely(prev != next)) {
#ifdef CONFIG_SMP
		this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
		this_cpu_write(cpu_tlbstate.active_mm, next);
#endif
		cpumask_set_cpu(cpu, mm_cpumask(next));
		load_cr3(next->pgd);
		trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);
		cpumask_clear_cpu(cpu, mm_cpumask(prev));
		load_mm_cr4(next);
#ifdef CONFIG_MODIFY_LDT_SYSCALL
		if (unlikely(prev->context.ldt != next->context.ldt))
			load_mm_ldt(next);
#endif
	}
#ifdef CONFIG_SMP
	  else {
		this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
		BUG_ON(this_cpu_read(cpu_tlbstate.active_mm) != next);
		if (!cpumask_test_cpu(cpu, mm_cpumask(next))) {
			cpumask_set_cpu(cpu, mm_cpumask(next));
			load_cr3(next->pgd);
			trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);
			load_mm_cr4(next);
			load_mm_ldt(next);
		}
	}
#endif
}