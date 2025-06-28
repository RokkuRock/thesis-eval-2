PJ_DEF(void) pj_scan_get_until_chr( pj_scanner *scanner,
				     const char *until_spec, pj_str_t *out)
{
    register char *s = scanner->curptr;
    pj_size_t speclen;
    if (s >= scanner->end) {
	pj_scan_syntax_err(scanner);
	return;
    }
    speclen = strlen(until_spec);
    while (PJ_SCAN_CHECK_EOF(s) && !memchr(until_spec, *s, speclen)) {
	++s;
    }
    pj_strset3(out, scanner->curptr, s);
    scanner->curptr = s;
    if (PJ_SCAN_IS_PROBABLY_SPACE(*s) && scanner->skip_ws) {
	pj_scan_skip_whitespace(scanner);
    }
}