static int futex_wait_requeue_pi(u32 __user *uaddr, int fshared,
				 u32 val, ktime_t *abs_time, u32 bitset,
				 int clockrt, u32 __user *uaddr2)
{
	struct hrtimer_sleeper timeout, *to = NULL;
	struct rt_mutex_waiter rt_waiter;
	struct rt_mutex *pi_mutex = NULL;
	struct futex_hash_bucket *hb;
	union futex_key key2;
	struct futex_q q;
	int res, ret;
	if (!bitset)
		return -EINVAL;
	if (abs_time) {
		to = &timeout;
		hrtimer_init_on_stack(&to->timer, clockrt ? CLOCK_REALTIME :
				      CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
		hrtimer_init_sleeper(to, current);
		hrtimer_set_expires_range_ns(&to->timer, *abs_time,
					     current->timer_slack_ns);
	}
	debug_rt_mutex_init_waiter(&rt_waiter);
	rt_waiter.task = NULL;
	key2 = FUTEX_KEY_INIT;
	ret = get_futex_key(uaddr2, fshared, &key2);
	if (unlikely(ret != 0))
		goto out;
	q.pi_state = NULL;
	q.bitset = bitset;
	q.rt_waiter = &rt_waiter;
	q.requeue_pi_key = &key2;
	ret = futex_wait_setup(uaddr, val, fshared, &q, &hb);
	if (ret)
		goto out_key2;
	futex_wait_queue_me(hb, &q, to);
	spin_lock(&hb->lock);
	ret = handle_early_requeue_pi_wakeup(hb, &q, &key2, to);
	spin_unlock(&hb->lock);
	if (ret)
		goto out_put_keys;
	if (!q.rt_waiter) {
		if (q.pi_state && (q.pi_state->owner != current)) {
			spin_lock(q.lock_ptr);
			ret = fixup_pi_state_owner(uaddr2, &q, current,
						   fshared);
			spin_unlock(q.lock_ptr);
		}
	} else {
		WARN_ON(!&q.pi_state);
		pi_mutex = &q.pi_state->pi_mutex;
		ret = rt_mutex_finish_proxy_lock(pi_mutex, to, &rt_waiter, 1);
		debug_rt_mutex_free_waiter(&rt_waiter);
		spin_lock(q.lock_ptr);
		res = fixup_owner(uaddr2, fshared, &q, !ret);
		if (res)
			ret = (res < 0) ? res : 0;
		unqueue_me_pi(&q);
	}
	if (ret == -EFAULT) {
		if (rt_mutex_owner(pi_mutex) == current)
			rt_mutex_unlock(pi_mutex);
	} else if (ret == -EINTR) {
		ret = -EWOULDBLOCK;
	}
out_put_keys:
	put_futex_key(fshared, &q.key);
out_key2:
	put_futex_key(fshared, &key2);
out:
	if (to) {
		hrtimer_cancel(&to->timer);
		destroy_hrtimer_on_stack(&to->timer);
	}
	return ret;
}