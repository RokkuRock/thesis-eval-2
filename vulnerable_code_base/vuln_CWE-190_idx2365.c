void common_timer_get(struct k_itimer *timr, struct itimerspec64 *cur_setting)
{
	const struct k_clock *kc = timr->kclock;
	ktime_t now, remaining, iv;
	struct timespec64 ts64;
	bool sig_none;
	sig_none = timr->it_sigev_notify == SIGEV_NONE;
	iv = timr->it_interval;
	if (iv) {
		cur_setting->it_interval = ktime_to_timespec64(iv);
	} else if (!timr->it_active) {
		if (!sig_none)
			return;
	}
	kc->clock_get(timr->it_clock, &ts64);
	now = timespec64_to_ktime(ts64);
	if (iv && (timr->it_requeue_pending & REQUEUE_PENDING || sig_none))
		timr->it_overrun += (int)kc->timer_forward(timr, now);
	remaining = kc->timer_remaining(timr, now);
	if (remaining <= 0) {
		if (!sig_none)
			cur_setting->it_value.tv_nsec = 1;
	} else {
		cur_setting->it_value = ktime_to_timespec64(remaining);
	}
}