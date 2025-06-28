parse_netscreen_packet(FILE_T fh, struct wtap_pkthdr *phdr, Buffer* buf,
    char *line, int *err, gchar **err_info)
{
	int		sec;
	int		dsec;
	char		cap_int[NETSCREEN_MAX_INT_NAME_LENGTH];
	char		direction[2];
	guint		pkt_len;
	char		cap_src[13];
	char		cap_dst[13];
	guint8		*pd;
	gchar		*p;
	int		n, i = 0;
	guint		offset = 0;
	gchar		dststr[13];
	phdr->rec_type = REC_TYPE_PACKET;
	phdr->presence_flags = WTAP_HAS_TS|WTAP_HAS_CAP_LEN;
	if (sscanf(line, "%9d.%9d: %15[a-z0-9/:.-](%1[io]) len=%9u:%12s->%12s/",
		   &sec, &dsec, cap_int, direction, &pkt_len, cap_src, cap_dst) < 5) {
		*err = WTAP_ERR_BAD_FILE;
		*err_info = g_strdup("netscreen: Can't parse packet-header");
		return -1;
	}
	if (pkt_len > WTAP_MAX_PACKET_SIZE) {
		*err = WTAP_ERR_BAD_FILE;
		*err_info = g_strdup_printf("netscreen: File has %u-byte packet, bigger than maximum of %u",
		    pkt_len, WTAP_MAX_PACKET_SIZE);
		return FALSE;
	}
	phdr->ts.secs  = sec;
	phdr->ts.nsecs = dsec * 100000000;
	phdr->len = pkt_len;
	ws_buffer_assure_space(buf, pkt_len);
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
		if (n == -1) {
			*err = WTAP_ERR_BAD_FILE;
			*err_info = g_strdup("netscreen: cannot parse hex-data");
			return FALSE;
		}
		offset += n;
		if (offset > pkt_len) {
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