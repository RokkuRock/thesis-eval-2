int common_timer_set(struct k_itimer *timr, int flags,
		     struct itimerspec64 *new_setting,
		     struct itimerspec64 *old_setting)
{
	const struct k_clock *kc = timr->kclock;
	bool sigev_none;
	ktime_t expires;
	if (old_setting)
		common_timer_get(timr, old_setting);
	timr->it_interval = 0;
	if (kc->timer_try_to_cancel(timr) < 0)
		return TIMER_RETRY;
	timr->it_active = 0;
	timr->it_requeue_pending = (timr->it_requeue_pending + 2) &
		~REQUEUE_PENDING;
	timr->it_overrun_last = 0;
	if (!new_setting->it_value.tv_sec && !new_setting->it_value.tv_nsec)
		return 0;
	timr->it_interval = timespec64_to_ktime(new_setting->it_interval);
	expires = timespec64_to_ktime(new_setting->it_value);
	sigev_none = (timr->it_sigev_notify & ~SIGEV_THREAD_ID) == SIGEV_NONE;
	kc->timer_arm(timr, expires, flags & TIMER_ABSTIME, sigev_none);
	timr->it_active = !sigev_none;
	return 0;
}