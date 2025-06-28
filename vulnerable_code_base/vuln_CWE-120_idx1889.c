PJ_DEF(void) pj_scan_get_n( pj_scanner *scanner,
			    unsigned N, pj_str_t *out)
{
    if (scanner->curptr + N > scanner->end) {
	pj_scan_syntax_err(scanner);
	return;
    }
    pj_strset(out, scanner->curptr, N);
    scanner->curptr += N;
    if (PJ_SCAN_IS_PROBABLY_SPACE(*scanner->curptr) && scanner->skip_ws) {
	pj_scan_skip_whitespace(scanner);
    }
}