static gboolean netscreen_read(wtap *wth, int *err, gchar **err_info,
    gint64 *data_offset)
{
	gint64		offset;
	int		pkt_len;
	char		line[NETSCREEN_LINE_LENGTH];
	char		cap_int[NETSCREEN_MAX_INT_NAME_LENGTH];
	gboolean	cap_dir;
	char		cap_dst[13];
	offset = netscreen_seek_next_packet(wth, err, err_info, line);
	if (offset < 0)
		return FALSE;
	pkt_len = parse_netscreen_rec_hdr(&wth->phdr, line, cap_int, &cap_dir,
	    cap_dst, err, err_info);
	if (pkt_len == -1)
		return FALSE;
	if (!parse_netscreen_hex_dump(wth->fh, pkt_len, cap_int,
	    cap_dst, &wth->phdr, wth->frame_buffer, err, err_info))
		return FALSE;
	if (wth->file_encap == WTAP_ENCAP_UNKNOWN)
		wth->file_encap = wth->phdr.pkt_encap;
	else {
		if (wth->file_encap != wth->phdr.pkt_encap)
			wth->file_encap = WTAP_ENCAP_PER_PACKET;
	}
	*data_offset = offset;
	return TRUE;
}