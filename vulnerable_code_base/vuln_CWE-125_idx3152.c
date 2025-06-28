parserep(netdissect_options *ndo,
         register const struct sunrpc_msg *rp, register u_int length)
{
	register const uint32_t *dp;
	u_int len;
	enum sunrpc_accept_stat astat;
	dp = ((const uint32_t *)&rp->rm_reply) + 1;
	ND_TCHECK(dp[1]);
	len = EXTRACT_32BITS(&dp[1]);
	if (len >= length)
		return (NULL);
	dp += (len + (2*sizeof(uint32_t) + 3)) / sizeof(uint32_t);
	ND_TCHECK2(dp[0], 0);
	astat = (enum sunrpc_accept_stat) EXTRACT_32BITS(dp);
	if (astat != SUNRPC_SUCCESS) {
		ND_PRINT((ndo, " %s", tok2str(sunrpc_str, "ar_stat %d", astat)));
		nfserr = 1;		 
		return (NULL);
	}
	ND_TCHECK2(*dp, sizeof(astat));
	return ((const uint32_t *) (sizeof(astat) + ((const char *)dp)));
trunc:
	return (0);
}