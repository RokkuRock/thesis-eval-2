PJ_DEF(void) pj_scan_get( pj_scanner *scanner,
			  const pj_cis_t *spec, pj_str_t *out)
{
    register char *s = scanner->curptr;
    pj_assert(pj_cis_match(spec,0)==0);
    if (!pj_cis_match(spec, *s)) {
	pj_scan_syntax_err(scanner);
	return;
    }
    do {
	++s;
    } while (pj_cis_match(spec, *s));
    pj_strset3(out, scanner->curptr, s);
    scanner->curptr = s;
    if (PJ_SCAN_IS_PROBABLY_SPACE(*s) && scanner->skip_ws) {
	pj_scan_skip_whitespace(scanner);    
    }
}