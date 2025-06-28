static void parse_version(pj_scanner *scanner, 
                          volatile parse_context *ctx)
{
    ctx->last_error = PJMEDIA_SDP_EINVER;
    if (*(scanner->curptr+1) != '=') {
	on_scanner_error(scanner);
	return;
    }
    if (*(scanner->curptr+2) != '0') {
	on_scanner_error(scanner);
	return;
    }
    pj_scan_skip_line(scanner);
}