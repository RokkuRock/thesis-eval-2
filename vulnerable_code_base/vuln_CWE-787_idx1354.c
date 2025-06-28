static void zep_print_ts(netdissect_options *ndo, const u_char *p)
{
	int32_t i;
	uint32_t uf;
	uint32_t f;
	float ff;
	i = GET_BE_U_4(p);
	uf = GET_BE_U_4(p + 4);
	ff = (float) uf;
	if (ff < 0.0)            
		ff += FMAXINT;
	ff = (float) (ff / FMAXINT);  
	f = (uint32_t) (ff * 1000000000.0);   
	ND_PRINT("%u.%09d", i, f);
	if (i) {
		time_t seconds = i - JAN_1970;
		struct tm *tm;
		char time_buf[128];
		tm = localtime(&seconds);
		strftime(time_buf, sizeof (time_buf), "%Y/%m/%d %H:%M:%S", tm);
		ND_PRINT(" (%s)", time_buf);
	}
}