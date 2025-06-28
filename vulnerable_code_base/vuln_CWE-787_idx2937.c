archive_string_append_from_wcs(struct archive_string *as,
    const wchar_t *w, size_t len)
{
	int n, ret_val = 0;
	char *p;
	char *end;
#if HAVE_WCRTOMB
	mbstate_t shift_state;
	memset(&shift_state, 0, sizeof(shift_state));
#else
	wctomb(NULL, L'\0');
#endif
	if (archive_string_ensure(as, as->length + len + 1) == NULL)
		return (-1);
	p = as->s + as->length;
	end = as->s + as->buffer_length - MB_CUR_MAX -1;
	while (*w != L'\0' && len > 0) {
		if (p >= end) {
			as->length = p - as->s;
			as->s[as->length] = '\0';
			if (archive_string_ensure(as,
			    as->length + len * 2 + 1) == NULL)
				return (-1);
			p = as->s + as->length;
			end = as->s + as->buffer_length - MB_CUR_MAX -1;
		}
#if HAVE_WCRTOMB
		n = wcrtomb(p, *w++, &shift_state);
#else
		n = wctomb(p, *w++);
#endif
		if (n == -1) {
			if (errno == EILSEQ) {
				*p++ = '?';
				ret_val = -1;
			} else {
				ret_val = -1;
				break;
			}
		} else
			p += n;
		len--;
	}
	as->length = p - as->s;
	as->s[as->length] = '\0';
	return (ret_val);
}