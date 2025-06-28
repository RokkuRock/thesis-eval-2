static void bfq_idle_slice_timer_body(struct bfq_queue *bfqq)
{
	struct bfq_data *bfqd = bfqq->bfqd;
	enum bfqq_expiration reason;
	unsigned long flags;
	spin_lock_irqsave(&bfqd->lock, flags);
	bfq_clear_bfqq_wait_request(bfqq);
	if (bfqq != bfqd->in_service_queue) {
		spin_unlock_irqrestore(&bfqd->lock, flags);
		return;
	}
	if (bfq_bfqq_budget_timeout(bfqq))
		reason = BFQQE_BUDGET_TIMEOUT;
	else if (bfqq->queued[0] == 0 && bfqq->queued[1] == 0)
		reason = BFQQE_TOO_IDLE;
	else
		goto schedule_dispatch;
	bfq_bfqq_expire(bfqd, bfqq, true, reason);
schedule_dispatch:
	spin_unlock_irqrestore(&bfqd->lock, flags);
	bfq_schedule_dispatch(bfqd);