parse_netscreen_hex_dump(FILE_T fh, int pkt_len, const char *cap_int,
    const char *cap_dst, struct wtap_pkthdr *phdr, Buffer* buf,
    int *err, gchar **err_info)
{
	guint8	*pd;
	gchar	line[NETSCREEN_LINE_LENGTH];
	gchar	*p;
	int	n, i = 0, offset = 0;
	gchar	dststr[13];
	ws_buffer_assure_space(buf, NETSCREEN_MAX_PACKET_LEN);
	pd = ws_buffer_start_ptr(buf);
	while(1) {
		if (file_gets(line, NETSCREEN_LINE_LENGTH, fh) == NULL) {
			break;
		}
		for (p = &line[0]; g_ascii_isspace(*p); p++)
			;
		if (*p == '\0') {
			break;
		}
		n = parse_single_hex_dump_line(p, pd, offset);
		if (offset == 0 && n < 6) {
			if (info_line(line)) {
				if (++i <= NETSCREEN_MAX_INFOLINES) {
					continue;
				}
			} else {
				*err = WTAP_ERR_BAD_FILE;
				*err_info = g_strdup("netscreen: cannot parse hex-data");
				return FALSE;
			}
		}
		if(n == -1) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = g_strdup("netscreen: cannot parse hex-data");
			return FALSE;
		}
		offset += n;
		if(offset > pkt_len) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = g_strdup("netscreen: too much hex-data");
			return FALSE;
		}
	}
	if (strncmp(cap_int, "adsl", 4) == 0) {
		g_snprintf(dststr, 13, "%02x%02x%02x%02x%02x%02x",
		   pd[0], pd[1], pd[2], pd[3], pd[4], pd[5]);
		if (strncmp(dststr, cap_dst, 12) == 0)
			phdr->pkt_encap = WTAP_ENCAP_ETHERNET;
		else
			phdr->pkt_encap = WTAP_ENCAP_PPP;
		}
	else if (strncmp(cap_int, "seri", 4) == 0)
		phdr->pkt_encap = WTAP_ENCAP_PPP;
	else
		phdr->pkt_encap = WTAP_ENCAP_ETHERNET;
	phdr->caplen = offset;
	return TRUE;
}