void set_task_cpu(struct task_struct *p, unsigned int new_cpu)
{
#ifdef CONFIG_SCHED_DEBUG
	WARN_ON_ONCE(p->state != TASK_RUNNING && p->state != TASK_WAKING &&
			!(task_thread_info(p)->preempt_count & PREEMPT_ACTIVE));
#ifdef CONFIG_LOCKDEP
	WARN_ON_ONCE(debug_locks && !(lockdep_is_held(&p->pi_lock) ||
				      lockdep_is_held(&task_rq(p)->lock)));
#endif
#endif
	trace_sched_migrate_task(p, new_cpu);
	if (task_cpu(p) != new_cpu) {
		p->se.nr_migrations++;
		perf_sw_event(PERF_COUNT_SW_CPU_MIGRATIONS, 1, 1, NULL, 0);
	}
	__set_task_cpu(p, new_cpu);
}