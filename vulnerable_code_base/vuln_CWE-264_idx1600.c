void perf_event_disable(struct perf_event *event)
{
	struct perf_event_context *ctx = event->ctx;
	struct task_struct *task = ctx->task;
	if (!task) {
		cpu_function_call(event->cpu, __perf_event_disable, event);
		return;
	}
retry:
	if (!task_function_call(task, __perf_event_disable, event))
		return;
	raw_spin_lock_irq(&ctx->lock);
	if (event->state == PERF_EVENT_STATE_ACTIVE) {
		raw_spin_unlock_irq(&ctx->lock);
		task = ctx->task;
		goto retry;
	}
	if (event->state == PERF_EVENT_STATE_INACTIVE) {
		update_group_times(event);
		event->state = PERF_EVENT_STATE_OFF;
	}
	raw_spin_unlock_irq(&ctx->lock);
}