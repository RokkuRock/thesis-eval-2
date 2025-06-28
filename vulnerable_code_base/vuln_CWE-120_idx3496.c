PJ_DEF(void) pj_scan_advance_n( pj_scanner *scanner,
				 unsigned N, pj_bool_t skip_ws)
{
    if (scanner->curptr + N > scanner->end) {
	pj_scan_syntax_err(scanner);
	return;
    }
    scanner->curptr += N;
    if (PJ_SCAN_IS_PROBABLY_SPACE(*scanner->curptr) && skip_ws) {
	pj_scan_skip_whitespace(scanner);
    }
}