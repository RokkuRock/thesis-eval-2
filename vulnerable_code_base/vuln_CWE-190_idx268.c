static enum entity_charset determine_charset(char *charset_hint TSRMLS_DC)
{
	int i;
	enum entity_charset charset = cs_utf_8;
	int len = 0;
	const zend_encoding *zenc;
	if (charset_hint == NULL)
		return cs_utf_8;
	if ((len = strlen(charset_hint)) != 0) {
		goto det_charset;
	}
	zenc = zend_multibyte_get_internal_encoding(TSRMLS_C);
	if (zenc != NULL) {
		charset_hint = (char *)zend_multibyte_get_encoding_name(zenc);
		if (charset_hint != NULL && (len=strlen(charset_hint)) != 0) {
			if ((len == 4)   &&
					(!memcmp("pass", charset_hint, 4) ||
					 !memcmp("auto", charset_hint, 4) ||
					 !memcmp("auto", charset_hint, 4))) {
				charset_hint = NULL;
				len = 0;
			} else {
				goto det_charset;
			}
		}
	}
	charset_hint = SG(default_charset);
	if (charset_hint != NULL && (len=strlen(charset_hint)) != 0) {
		goto det_charset;
	}
#if HAVE_NL_LANGINFO && HAVE_LOCALE_H && defined(CODESET)
	charset_hint = nl_langinfo(CODESET);
	if (charset_hint != NULL && (len=strlen(charset_hint)) != 0) {
		goto det_charset;
	}
#endif
#if HAVE_LOCALE_H
	{
		char *localename;
		char *dot, *at;
		localename = setlocale(LC_CTYPE, NULL);
		dot = strchr(localename, '.');
		if (dot) {
			dot++;
			at = strchr(dot, '@');
			if (at)
				len = at - dot;
			else
				len = strlen(dot);
			charset_hint = dot;
		} else {
			charset_hint = localename;
			len = strlen(charset_hint);
		}
	}
#endif
det_charset:
	if (charset_hint) {
		int found = 0;
		for (i = 0; charset_map[i].codeset; i++) {
			if (len == strlen(charset_map[i].codeset) && strncasecmp(charset_hint, charset_map[i].codeset, len) == 0) {
				charset = charset_map[i].charset;
				found = 1;
				break;
			}
		}
		if (!found) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "charset `%s' not supported, assuming utf-8",
					charset_hint);
		}
	}
	return charset;
}