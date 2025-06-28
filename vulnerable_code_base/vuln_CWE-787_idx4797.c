arista_print_date_hms_time(netdissect_options *ndo, uint32_t seconds,
		uint32_t nanoseconds)
{
	time_t ts;
	struct tm *tm;
	char buf[BUFSIZE];
	ts = seconds + (nanoseconds / 1000000000);
	nanoseconds %= 1000000000;
	if (NULL == (tm = gmtime(&ts)))
		ND_PRINT("gmtime() error");
	else if (0 == strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm))
		ND_PRINT("strftime() error");
	else
		ND_PRINT("%s.%09u", buf, nanoseconds);
}