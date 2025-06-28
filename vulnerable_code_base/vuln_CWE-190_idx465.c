static void traverse_for_entities(
	const char *old,
	size_t oldlen,
	char *ret,  
	size_t *retlen,
	int all,
	int flags,
	const entity_ht *inv_map,
	enum entity_charset charset)
{
	const char *p,
			   *lim;
	char	   *q;
	int doctype = flags & ENT_HTML_DOC_TYPE_MASK;
	lim = old + oldlen;  
	assert(*lim == '\0');
	for (p = old, q = ret; p < lim;) {
		unsigned code, code2 = 0;
		const char *next = NULL;  
		if (p[0] != '&' || (p + 3 >= lim)) {
			*(q++) = *(p++);
			continue;
		}
		if (p[1] == '#') {
			next = &p[2];
			if (process_numeric_entity(&next, &code) == FAILURE)
				goto invalid_code;
			if (!all && (code > 63U ||
					stage3_table_be_apos_00000[code].data.ent.entity == NULL))
				goto invalid_code;
			if (!unicode_cp_is_allowed(code, doctype) ||
					(doctype == ENT_HTML_DOC_HTML5 && code == 0x0D))
				goto invalid_code;
		} else {
			const char *start;
			size_t ent_len;
			next = &p[1];
			start = next;
			if (process_named_entity_html(&next, &start, &ent_len) == FAILURE)
				goto invalid_code;
			if (resolve_named_entity_html(start, ent_len, inv_map, &code, &code2) == FAILURE) {
				if (doctype == ENT_HTML_DOC_XHTML && ent_len == 4 && start[0] == 'a'
							&& start[1] == 'p' && start[2] == 'o' && start[3] == 's') {
					code = (unsigned) '\'';
				} else {
					goto invalid_code;
				}
			}
		}
		assert(*next == ';');
		if (((code == '\'' && !(flags & ENT_HTML_QUOTE_SINGLE)) ||
				(code == '"' && !(flags & ENT_HTML_QUOTE_DOUBLE)))
				 )
			goto invalid_code;
		if (charset != cs_utf_8) {
			if (map_from_unicode(code, charset, &code) == FAILURE || code2 != 0)
				goto invalid_code;  
		}
		q += write_octet_sequence(q, charset, code);
		if (code2) {
			q += write_octet_sequence(q, charset, code2);
		}
		p = next + 1;
		continue;
invalid_code:
		for (; p < next; p++) {
			*(q++) = *p;
		}
	}
	*q = '\0';
	*retlen = (size_t)(q - ret);
}