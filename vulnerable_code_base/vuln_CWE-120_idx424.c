PJ_DEF(void) pj_scan_get_until_ch( pj_scanner *scanner, 
				   int until_char, pj_str_t *out)
{
    register char *s = scanner->curptr;
    if (s >= scanner->end) {
	pj_scan_syntax_err(scanner);
	return;
    }
    while (PJ_SCAN_CHECK_EOF(s) && *s != until_char) {
	++s;
    }
    pj_strset3(out, scanner->curptr, s);
    scanner->curptr = s;
    if (PJ_SCAN_IS_PROBABLY_SPACE(*s) && scanner->skip_ws) {
	pj_scan_skip_whitespace(scanner);
    }
}