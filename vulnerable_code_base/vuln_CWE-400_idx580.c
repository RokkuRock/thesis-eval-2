static int do_sched_cfs_period_timer(struct cfs_bandwidth *cfs_b, int overrun, unsigned long flags)
{
	u64 runtime, runtime_expires;
	int throttled;
	if (cfs_b->quota == RUNTIME_INF)
		goto out_deactivate;
	throttled = !list_empty(&cfs_b->throttled_cfs_rq);
	cfs_b->nr_periods += overrun;
	if (cfs_b->idle && !throttled)
		goto out_deactivate;
	__refill_cfs_bandwidth_runtime(cfs_b);
	if (!throttled) {
		cfs_b->idle = 1;
		return 0;
	}
	cfs_b->nr_throttled += overrun;
	runtime_expires = cfs_b->runtime_expires;
	while (throttled && cfs_b->runtime > 0 && !cfs_b->distribute_running) {
		runtime = cfs_b->runtime;
		cfs_b->distribute_running = 1;
		raw_spin_unlock_irqrestore(&cfs_b->lock, flags);
		runtime = distribute_cfs_runtime(cfs_b, runtime,
						 runtime_expires);
		raw_spin_lock_irqsave(&cfs_b->lock, flags);
		cfs_b->distribute_running = 0;
		throttled = !list_empty(&cfs_b->throttled_cfs_rq);
		lsub_positive(&cfs_b->runtime, runtime);
	}
	cfs_b->idle = 0;
	return 0;
out_deactivate:
	return 1;
}