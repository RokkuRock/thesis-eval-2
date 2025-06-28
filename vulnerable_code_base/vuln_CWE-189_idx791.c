jiffies_to_compat_timeval(unsigned long jiffies, struct compat_timeval *value)
{
	u64 nsec = (u64)jiffies * TICK_NSEC;
	long rem;
	value->tv_sec = div_long_long_rem(nsec, NSEC_PER_SEC, &rem);
	value->tv_usec = rem / NSEC_PER_USEC;
}