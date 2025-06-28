atm_if_print(netdissect_options *ndo,
             const struct pcap_pkthdr *h, const u_char *p)
{
	u_int caplen = h->caplen;
	u_int length = h->len;
	uint32_t llchdr;
	u_int hdrlen = 0;
	if (caplen < 1 || length < 1) {
		ND_PRINT((ndo, "%s", tstr));
		return (caplen);
	}
        if (*p == LLC_UI) {
            if (ndo->ndo_eflag)
                ND_PRINT((ndo, "CNLPID "));
            isoclns_print(ndo, p + 1, length - 1, caplen - 1);
            return hdrlen;
        }
	if (caplen < 3 || length < 3) {
		ND_PRINT((ndo, "%s", tstr));
		return (caplen);
	}
	llchdr = EXTRACT_24BITS(p);
	if (llchdr != LLC_UI_HDR(LLCSAP_SNAP) &&
	    llchdr != LLC_UI_HDR(LLCSAP_ISONS) &&
	    llchdr != LLC_UI_HDR(LLCSAP_IP)) {
		if (caplen < 20 || length < 20) {
			ND_PRINT((ndo, "%s", tstr));
			return (caplen);
		}
		if (ndo->ndo_eflag)
			ND_PRINT((ndo, "%08x%08x %08x%08x ",
			       EXTRACT_32BITS(p),
			       EXTRACT_32BITS(p+4),
			       EXTRACT_32BITS(p+8),
			       EXTRACT_32BITS(p+12)));
		p += 20;
		length -= 20;
		caplen -= 20;
		hdrlen += 20;
	}
	hdrlen += atm_llc_print(ndo, p, length, caplen);
	return (hdrlen);
}