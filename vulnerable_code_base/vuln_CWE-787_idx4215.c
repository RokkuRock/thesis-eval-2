ts_date_hmsfrac_print(netdissect_options *ndo, long sec, long usec,
		      enum date_flag date_flag, enum time_flag time_flag)
{
	time_t Time = sec;
	struct tm *tm;
	char timestr[32];
	if ((unsigned)sec & 0x80000000) {
		ND_PRINT("[Error converting time]");
		return;
	}
	if (time_flag == LOCAL_TIME)
		tm = localtime(&Time);
	else
		tm = gmtime(&Time);
	if (!tm) {
		ND_PRINT("[Error converting time]");
		return;
	}
	if (date_flag == WITH_DATE)
		strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm);
	else
		strftime(timestr, sizeof(timestr), "%H:%M:%S", tm);
	ND_PRINT("%s", timestr);
	ts_frac_print(ndo, usec);
}