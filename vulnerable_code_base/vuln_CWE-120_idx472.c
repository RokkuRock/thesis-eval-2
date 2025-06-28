PJ_DEF(void) pj_scan_get_unescape( pj_scanner *scanner,
				   const pj_cis_t *spec, pj_str_t *out)
{
    register char *s = scanner->curptr;
    char *dst = s;
    pj_assert(pj_cis_match(spec,0)==0);
    pj_assert(pj_cis_match(spec,'%')==0);
    if (!pj_cis_match(spec, *s) && *s != '%') {
	pj_scan_syntax_err(scanner);
	return;
    }
    out->ptr = s;
    do {
	if (*s == '%') {
	    if (s+3 <= scanner->end && pj_isxdigit(*(s+1)) && 
		pj_isxdigit(*(s+2))) 
	    {
		*dst = (pj_uint8_t) ((pj_hex_digit_to_val(*(s+1)) << 4) +
				      pj_hex_digit_to_val(*(s+2)));
		++dst;
		s += 3;
	    } else {
		*dst++ = *s++;
		*dst++ = *s++;
		break;
	    }
	}
	if (pj_cis_match(spec, *s)) {
	    char *start = s;
	    do {
		++s;
	    } while (pj_cis_match(spec, *s));
	    if (dst != start) pj_memmove(dst, start, s-start);
	    dst += (s-start);
	} 
    } while (*s == '%');
    scanner->curptr = s;
    out->slen = (dst - out->ptr);
    if (PJ_SCAN_IS_PROBABLY_SPACE(*s) && scanner->skip_ws) {
	pj_scan_skip_whitespace(scanner);    
    }
}