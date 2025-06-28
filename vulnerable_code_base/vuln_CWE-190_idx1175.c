PHPAPI char *php_escape_html_entities_ex(unsigned char *old, size_t oldlen, size_t *newlen, int all, int flags, char *hint_charset, zend_bool double_encode TSRMLS_DC)
{
	size_t cursor, maxlen, len;
	char *replaced;
	enum entity_charset charset = determine_charset(hint_charset TSRMLS_CC);
	int doctype = flags & ENT_HTML_DOC_TYPE_MASK;
	entity_table_opt entity_table;
	const enc_to_uni *to_uni_table = NULL;
	const entity_ht *inv_map = NULL;  
	const unsigned char *replacement = NULL;
	size_t replacement_len = 0;
	if (all) {  
		if (CHARSET_PARTIAL_SUPPORT(charset)) {
			php_error_docref0(NULL TSRMLS_CC, E_STRICT, "Only basic entities "
				"substitution is supported for multi-byte encodings other than UTF-8; "
				"functionality is equivalent to htmlspecialchars");
		}
		LIMIT_ALL(all, doctype, charset);
	}
	entity_table = determine_entity_table(all, doctype);
	if (all && !CHARSET_UNICODE_COMPAT(charset)) {
		to_uni_table = enc_to_uni_index[charset];
	}
	if (!double_encode) {
		inv_map = unescape_inverse_map(1, flags);
	}
	if (flags & (ENT_HTML_SUBSTITUTE_ERRORS | ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS)) {
		if (charset == cs_utf_8) {
			replacement = (const unsigned char*)"\xEF\xBF\xBD";
			replacement_len = sizeof("\xEF\xBF\xBD") - 1;
		} else {
			replacement = (const unsigned char*)"&#xFFFD;";
			replacement_len = sizeof("&#xFFFD;") - 1;
		}
	}
	if (oldlen < 64) {
		maxlen = 128;	
	} else {
		maxlen = 2 * oldlen;
		if (maxlen < oldlen) {
			zend_error_noreturn(E_ERROR, "Input string is too long");
			return NULL;
		}
	}
	replaced = emalloc(maxlen + 1);  
	len = 0;
	cursor = 0;
	while (cursor < oldlen) {
		const unsigned char *mbsequence = NULL;
		size_t mbseqlen					= 0,
		       cursor_before			= cursor;
		int status						= SUCCESS;
		unsigned int this_char			= get_next_char(charset, old, oldlen, &cursor, &status);
		if (len > maxlen - 40) {  
			replaced = safe_erealloc(replaced, maxlen , 1, 128 + 1);
			maxlen += 128;
		}
		if (status == FAILURE) {
			if (flags & ENT_HTML_IGNORE_ERRORS) {
				continue;
			} else if (flags & ENT_HTML_SUBSTITUTE_ERRORS) {
				memcpy(&replaced[len], replacement, replacement_len);
				len += replacement_len;
				continue;
			} else {
				efree(replaced);
				*newlen = 0;
				return STR_EMPTY_ALLOC();
			}
		} else {  
			mbsequence = &old[cursor_before];
			mbseqlen = cursor - cursor_before;
		}
		if (this_char != '&') {  
			const unsigned char *rep	= NULL;
			size_t				rep_len	= 0;
			if (((this_char == '\'' && !(flags & ENT_HTML_QUOTE_SINGLE)) ||
					(this_char == '"' && !(flags & ENT_HTML_QUOTE_DOUBLE))))
				goto pass_char_through;
			if (all) {  
				if (to_uni_table != NULL) {
					map_to_unicode(this_char, to_uni_table, &this_char);
					if (this_char == 0xFFFF)  
						goto pass_char_through;
				}
				find_entity_for_char(this_char, charset, entity_table.ms_table, &rep,
					&rep_len, old, oldlen, &cursor);
			} else {
				find_entity_for_char_basic(this_char, entity_table.table, &rep, &rep_len);
			}
			if (rep != NULL) {
				replaced[len++] = '&';
				memcpy(&replaced[len], rep, rep_len);
				len += rep_len;
				replaced[len++] = ';';
			} else {
				if (flags & ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS) {
					if (CHARSET_UNICODE_COMPAT(charset)) {
						if (!unicode_cp_is_allowed(this_char, doctype)) {
							mbsequence = replacement;
							mbseqlen = replacement_len;
						}
					} else if (to_uni_table) {
						if (!all)  
							map_to_unicode(this_char, to_uni_table, &this_char);
						if (!unicode_cp_is_allowed(this_char, doctype)) {
							mbsequence = replacement;
							mbseqlen = replacement_len;
						}
					} else {
						if (this_char <= 0x7D &&
								!unicode_cp_is_allowed(this_char, doctype)) {
							mbsequence = replacement;
							mbseqlen = replacement_len;
						}
					}
				}
pass_char_through:
				if (mbseqlen > 1) {
					memcpy(replaced + len, mbsequence, mbseqlen);
					len += mbseqlen;
				} else {
					replaced[len++] = mbsequence[0];
				}
			}
		} else {  
			if (double_encode) {
encode_amp:
				memcpy(&replaced[len], "&amp;", sizeof("&amp;") - 1);
				len += sizeof("&amp;") - 1;
			} else {  
				size_t ent_len;  
				if (old[cursor] == '#') {  
					unsigned code_point;
					int valid;
					char *pos = (char*)&old[cursor+1];
					valid = process_numeric_entity((const char **)&pos, &code_point);
					if (valid == FAILURE)
						goto encode_amp;
					if (flags & ENT_HTML_SUBSTITUTE_DISALLOWED_CHARS) {
						if (!numeric_entity_is_allowed(code_point, doctype))
							goto encode_amp;
					}
					ent_len = pos - (char*)&old[cursor];
				} else {  
					const char *start = &old[cursor],
							   *next = start;
					unsigned   dummy1, dummy2;
					if (process_named_entity_html(&next, &start, &ent_len) == FAILURE)
						goto encode_amp;
					if (resolve_named_entity_html(start, ent_len, inv_map, &dummy1, &dummy2) == FAILURE) {
						if (!(doctype == ENT_HTML_DOC_XHTML && ent_len == 4 && start[0] == 'a'
									&& start[1] == 'p' && start[2] == 'o' && start[3] == 's')) {
							goto encode_amp;
						}
					}
				}
				if (maxlen - len < ent_len + 2  ) {
					replaced = safe_erealloc(replaced, maxlen, 1, ent_len + 128 + 1);
					maxlen += ent_len + 128;
				}
				replaced[len++] = '&';
				memcpy(&replaced[len], &old[cursor], ent_len);
				len += ent_len;
				replaced[len++] = ';';
				cursor += ent_len + 1;
			}
		}
	}
	replaced[len] = '\0';
	*newlen = len;
	return replaced;
}