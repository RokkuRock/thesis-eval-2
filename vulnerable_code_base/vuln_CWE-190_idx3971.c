static enum hrtimer_restart posix_timer_fn(struct hrtimer *timer)
{
	struct k_itimer *timr;
	unsigned long flags;
	int si_private = 0;
	enum hrtimer_restart ret = HRTIMER_NORESTART;
	timr = container_of(timer, struct k_itimer, it.real.timer);
	spin_lock_irqsave(&timr->it_lock, flags);
	timr->it_active = 0;
	if (timr->it_interval != 0)
		si_private = ++timr->it_requeue_pending;
	if (posix_timer_event(timr, si_private)) {
		if (timr->it_interval != 0) {
			ktime_t now = hrtimer_cb_get_time(timer);
#ifdef CONFIG_HIGH_RES_TIMERS
			{
				ktime_t kj = NSEC_PER_SEC / HZ;
				if (timr->it_interval < kj)
					now = ktime_add(now, kj);
			}
#endif
			timr->it_overrun += (unsigned int)
				hrtimer_forward(timer, now,
						timr->it_interval);
			ret = HRTIMER_RESTART;
			++timr->it_requeue_pending;
			timr->it_active = 1;
		}
	}
	unlock_timer(timr, flags);
	return ret;
}