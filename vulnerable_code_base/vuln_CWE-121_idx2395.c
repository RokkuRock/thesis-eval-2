static GF_Err xml_sax_parse(GF_SAXParser *parser, Bool force_parse)
{
	u32 i = 0;
	Bool is_text;
	u32 is_end;
	u8 c;
	char *elt, sep;
	u32 cdata_sep;
	while (parser->current_pos<parser->line_size) {
		if (!force_parse && parser->suspended) goto exit;
restart:
		is_text = GF_FALSE;
		switch (parser->sax_state) {
		case SAX_STATE_TEXT_CONTENT:
			is_text = GF_TRUE;
		case SAX_STATE_ELEMENT:
			elt = NULL;
			i=0;
			while ((c = parser->buffer[parser->current_pos+i]) !='<') {
				if ((parser->init_state==2) && (c ==']')) {
					parser->sax_state = SAX_STATE_ATT_NAME;
					parser->current_pos+=i+1;
					goto restart;
				}
				i++;
				if (c=='\n') parser->line++;
				if (is_text) {
					if (c=='&') parser->text_check_escapes |= 1;
					else if (c==';') parser->text_check_escapes |= 2;
				}
				if (parser->current_pos+i==parser->line_size) {
					if ((parser->line_size>=2*XML_INPUT_SIZE) && !parser->init_state)
						parser->sax_state = SAX_STATE_SYNTAX_ERROR;
					goto exit;
				}
			}
			if (is_text && i) {
				u32 has_esc = parser->text_check_escapes;
				xml_sax_store_text(parser, i);
				parser->text_check_escapes = has_esc;
				parser->sax_state = SAX_STATE_ELEMENT;
			} else if (i) {
				parser->current_pos += i;
				assert(parser->current_pos < parser->line_size);
			}
			is_end = 0;
			i = 0;
			cdata_sep = 0;
			while (1) {
				c = parser->buffer[parser->current_pos+1+i];
				if (!strncmp(parser->buffer+parser->current_pos+1+i, "!--", 3)) {
					parser->sax_state = SAX_STATE_COMMENT;
					i += 3;
					break;
				}
				if (!c) {
					goto exit;
				}
				if ((c=='\t') || (c=='\r') || (c==' ') ) {
					if (i) break;
					else parser->current_pos++;
				}
				else if (c=='\n') {
					parser->line++;
					if (i) break;
					else parser->current_pos++;
				}
				else if (c=='>') break;
				else if (c=='=') break;
				else if (c=='[') {
					i++;
					if (!cdata_sep) cdata_sep = 1;
					else {
						break;
					}
				}
				else if (c=='/') {
					is_end = !i ? 1 : 2;
					i++;
				} else if (c=='<') {
					if (parser->sax_state != SAX_STATE_COMMENT) {
						parser->sax_state = SAX_STATE_SYNTAX_ERROR;
						return GF_CORRUPTED_DATA;
					}
				} else {
					i++;
				}
				if (parser->current_pos+1+i==parser->line_size) {
					goto exit;
				}
			}
			if (i) {
				parser->elt_name_start = parser->current_pos+1 + 1;
				if (is_end==1) parser->elt_name_start ++;
				if (is_end==2) parser->elt_name_end = parser->current_pos+1+i;
				else parser->elt_name_end = parser->current_pos+1+i + 1;
			}
			if (is_end) {
				xml_sax_flush_text(parser);
				parser->elt_end_pos = parser->file_pos + parser->current_pos + i;
				if (is_end==2) {
					parser->sax_state = SAX_STATE_ELEMENT;
					xml_sax_node_start(parser);
					xml_sax_node_end(parser, GF_FALSE);
				} else {
					parser->elt_end_pos += parser->elt_name_end - parser->elt_name_start;
					xml_sax_node_end(parser, GF_TRUE);
				}
				if (parser->sax_state == SAX_STATE_SYNTAX_ERROR) break;
				parser->current_pos+=2+i;
				parser->sax_state = SAX_STATE_TEXT_CONTENT;
				break;
			}
			if (!parser->elt_name_end) {
				return GF_CORRUPTED_DATA;
			}
			sep = parser->buffer[parser->elt_name_end-1];
			parser->buffer[parser->elt_name_end-1] = 0;
			elt = parser->buffer + parser->elt_name_start-1;
			parser->sax_state = SAX_STATE_ATT_NAME;
			assert(parser->elt_start_pos <= parser->file_pos + parser->current_pos);
			parser->elt_start_pos = parser->file_pos + parser->current_pos;
			if (!strncmp(elt, "!--", 3)) {
				xml_sax_flush_text(parser);
				parser->sax_state = SAX_STATE_COMMENT;
				if (i>3) parser->current_pos -= (i-3);
			}
			else if (!strcmp(elt, "?xml")) parser->init_state = 1;
			else if (!strcmp(elt, "!DOCTYPE")) parser->init_state = 2;
			else if (!strcmp(elt, "!ENTITY")) parser->sax_state = SAX_STATE_ENTITY;
			else if (!strcmp(elt, "!ATTLIST") || !strcmp(elt, "!ELEMENT")) parser->sax_state = SAX_STATE_SKIP_DOCTYPE;
			else if (!strcmp(elt, "![CDATA["))
				parser->sax_state = SAX_STATE_CDATA;
			else if (elt[0]=='?') {
				i--;
				parser->sax_state = SAX_STATE_XML_PROC;
			}
			else {
				xml_sax_flush_text(parser);
				if (parser->init_state) {
					parser->init_state = 0;
					if (gf_list_count(parser->entities)) {
						char *orig_buf;
						GF_Err e;
						parser->buffer[parser->elt_name_end-1] = sep;
						orig_buf = gf_strdup(parser->buffer + parser->current_pos);
						parser->current_pos = 0;
						parser->line_size = 0;
						parser->elt_start_pos = 0;
						parser->sax_state = SAX_STATE_TEXT_CONTENT;
						e = gf_xml_sax_parse_intern(parser, orig_buf);
						gf_free(orig_buf);
						return e;
					}
				}
			}
			parser->current_pos+=1+i;
			parser->buffer[parser->elt_name_end-1] = sep;
			break;
		case SAX_STATE_COMMENT:
			if (!xml_sax_parse_comments(parser)) {
				xml_sax_swap(parser);
				goto exit;
			}
			break;
		case SAX_STATE_ATT_NAME:
		case SAX_STATE_ATT_VALUE:
			if (xml_sax_parse_attribute(parser))
				goto exit;
			break;
		case SAX_STATE_ENTITY:
			xml_sax_parse_entity(parser);
			break;
		case SAX_STATE_SKIP_DOCTYPE:
			xml_sax_skip_doctype(parser);
			break;
		case SAX_STATE_XML_PROC:
			xml_sax_skip_xml_proc(parser);
			break;
		case SAX_STATE_CDATA:
			xml_sax_cdata(parser);
			break;
		case SAX_STATE_SYNTAX_ERROR:
			return GF_CORRUPTED_DATA;
		case SAX_STATE_ALLOC_ERROR:
			return GF_OUT_OF_MEM;
		case SAX_STATE_DONE:
			return GF_EOS;
		}
	}
exit:
#if 0
	if (is_text) {
		if (i) xml_sax_store_text(parser, i);
	}
#endif
	xml_sax_swap(parser);
	if (parser->sax_state==SAX_STATE_SYNTAX_ERROR)
		return GF_CORRUPTED_DATA;
	else
		return GF_OK;
}