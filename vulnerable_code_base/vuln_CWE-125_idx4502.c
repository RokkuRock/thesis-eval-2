ikev2_sa_print(netdissect_options *ndo, u_char tpay,
		const struct isakmp_gen *ext1,
		u_int osa_length, const u_char *ep,
		uint32_t phase _U_, uint32_t doi _U_,
		uint32_t proto _U_, int depth)
{
	const struct isakmp_gen *ext;
	struct isakmp_gen e;
	u_int sa_length;
	const u_char *cp;
	int i;
	int pcount;
	u_char np;
	u_int item_len;
	ND_TCHECK(*ext1);
	UNALIGNED_MEMCPY(&e, ext1, sizeof(e));
	ikev2_pay_print(ndo, "sa", e.critical);
	osa_length= ntohs(e.len);
	sa_length = osa_length - 4;
	ND_PRINT((ndo," len=%d", sa_length));
	cp = (const u_char *)(ext1 + 1);
	pcount = 0;
	for (np = ISAKMP_NPTYPE_P; np != 0; np = e.np) {
		pcount++;
		ext = (const struct isakmp_gen *)cp;
		if (sa_length < sizeof(*ext))
			goto toolong;
		ND_TCHECK(*ext);
		UNALIGNED_MEMCPY(&e, ext, sizeof(e));
		item_len = ntohs(e.len);
		if (item_len <= 4)
			goto trunc;
		if (sa_length < item_len)
			goto toolong;
		ND_TCHECK2(*cp, item_len);
		depth++;
		ND_PRINT((ndo,"\n"));
		for (i = 0; i < depth; i++)
			ND_PRINT((ndo,"    "));
		ND_PRINT((ndo,"("));
		if (np == ISAKMP_NPTYPE_P) {
			cp = ikev2_p_print(ndo, np, pcount, ext, item_len,
					   ep, depth);
			if (cp == NULL) {
				return NULL;
			}
		} else {
			ND_PRINT((ndo, "%s", NPSTR(np)));
			cp += item_len;
		}
		ND_PRINT((ndo,")"));
		depth--;
		sa_length -= item_len;
	}
	return cp;
toolong:
	cp += sa_length;
	ND_PRINT((ndo," [|%s]", NPSTR(tpay)));
	return cp;
trunc:
	ND_PRINT((ndo," [|%s]", NPSTR(tpay)));
	return NULL;
}