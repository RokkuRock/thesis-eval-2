jiffies_to_timespec(const unsigned long jiffies, struct timespec *value)
{
	u64 nsec = (u64)jiffies * TICK_NSEC;
	value->tv_sec = div_long_long_rem(nsec, NSEC_PER_SEC, &value->tv_nsec);
}