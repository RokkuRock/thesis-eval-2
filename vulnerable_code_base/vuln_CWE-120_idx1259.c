static void parse_origin(pj_scanner *scanner, pjmedia_sdp_session *ses,
			 volatile parse_context *ctx)
{
    pj_str_t str;
    ctx->last_error = PJMEDIA_SDP_EINORIGIN;
    if (*(scanner->curptr+1) != '=') {
	on_scanner_error(scanner);
	return;
    }
    pj_scan_advance_n(scanner, 2, SKIP_WS);
    pj_scan_get_until_ch(scanner, ' ', &ses->origin.user);
    pj_scan_get_char(scanner);
    pj_scan_get_until_ch(scanner, ' ', &str);
    ses->origin.id = pj_strtoul(&str);
    pj_scan_get_char(scanner);
    pj_scan_get_until_ch(scanner, ' ', &str);
    ses->origin.version = pj_strtoul(&str);
    pj_scan_get_char(scanner);
    pj_scan_get_until_ch(scanner, ' ', &ses->origin.net_type);
    pj_scan_get_char(scanner);
    pj_scan_get_until_ch(scanner, ' ', &ses->origin.addr_type);
    pj_scan_get_char(scanner);
    pj_scan_get_until_chr(scanner, " \t\r\n", &ses->origin.addr);
    pj_scan_skip_line(scanner);
}