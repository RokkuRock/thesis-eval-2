 */
static enum hrtimer_restart bfq_idle_slice_timer(struct hrtimer *timer)
{
	struct bfq_data *bfqd = container_of(timer, struct bfq_data,
					     idle_slice_timer);
	struct bfq_queue *bfqq = bfqd->in_service_queue;
	if (bfqq)
		bfq_idle_slice_timer_body(bfqq);
	return HRTIMER_NORESTART;