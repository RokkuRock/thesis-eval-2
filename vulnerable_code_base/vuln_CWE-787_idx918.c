ahcp_time_print(netdissect_options *ndo,
                const u_char *cp, uint8_t len)
{
	time_t t;
	struct tm *tm;
	char buf[BUFSIZE];
	if (len != 4)
		goto invalid;
	t = GET_BE_U_4(cp);
	if (NULL == (tm = gmtime(&t)))
		ND_PRINT(": gmtime() error");
	else if (0 == strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm))
		ND_PRINT(": strftime() error");
	else
		ND_PRINT(": %s UTC", buf);
	return;
invalid:
	nd_print_invalid(ndo);
	ND_TCHECK_LEN(cp, len);
}