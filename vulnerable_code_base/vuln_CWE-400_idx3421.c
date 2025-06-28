static void expire_cfs_rq_runtime(struct cfs_rq *cfs_rq)
{
	struct cfs_bandwidth *cfs_b = tg_cfs_bandwidth(cfs_rq->tg);
	if (likely((s64)(rq_clock(rq_of(cfs_rq)) - cfs_rq->runtime_expires) < 0))
		return;
	if (cfs_rq->runtime_remaining < 0)
		return;
	if (cfs_rq->expires_seq == cfs_b->expires_seq) {
		cfs_rq->runtime_expires += TICK_NSEC;
	} else {
		cfs_rq->runtime_remaining = 0;
	}
}