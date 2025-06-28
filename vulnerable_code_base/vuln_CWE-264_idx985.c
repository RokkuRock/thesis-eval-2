void perf_event_enable(struct perf_event *event)
{
	struct perf_event_context *ctx = event->ctx;
	struct task_struct *task = ctx->task;
	if (!task) {
		cpu_function_call(event->cpu, __perf_event_enable, event);
		return;
	}
	raw_spin_lock_irq(&ctx->lock);
	if (event->state >= PERF_EVENT_STATE_INACTIVE)
		goto out;
	if (event->state == PERF_EVENT_STATE_ERROR)
		event->state = PERF_EVENT_STATE_OFF;
retry:
	if (!ctx->is_active) {
		__perf_event_mark_enabled(event);
		goto out;
	}
	raw_spin_unlock_irq(&ctx->lock);
	if (!task_function_call(task, __perf_event_enable, event))
		return;
	raw_spin_lock_irq(&ctx->lock);
	if (ctx->is_active && event->state == PERF_EVENT_STATE_OFF) {
		task = ctx->task;
		goto retry;
	}
out:
	raw_spin_unlock_irq(&ctx->lock);
}