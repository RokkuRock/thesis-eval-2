static inline void perf_event_task_sched_out(struct task_struct *task, struct task_struct *next)
{
	perf_sw_event(PERF_COUNT_SW_CONTEXT_SWITCHES, 1, 1, NULL, 0);
	__perf_event_task_sched_out(task, next);
}