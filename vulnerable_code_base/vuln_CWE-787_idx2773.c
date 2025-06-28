void gf_bt_check_line(GF_BTParser *parser)
{
	while (1) {
		switch (parser->line_buffer[parser->line_pos]) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			parser->line_pos++;
			continue;
		default:
			break;
		}
		break;
	}
	if (parser->line_buffer[parser->line_pos]=='#') {
		parser->line_size = parser->line_pos;
	}
	else if ((parser->line_buffer[parser->line_pos]=='/') && (parser->line_buffer[parser->line_pos+1]=='/') ) parser->line_size = parser->line_pos;
	if (parser->line_size == parser->line_pos) {
		if (!parser->gz_in) {
			parser->done = 1;
			return;
		}
next_line:
		parser->line_start_pos = (s32) gf_gztell(parser->gz_in);
		parser->line_buffer[0] = 0;
		if (parser->unicode_type) {
			u8 c1, c2;
			unsigned short wchar;
			unsigned short l[BT_LINE_SIZE];
			unsigned short *dst = l;
			Bool is_ret = 0;
			u32 last_space_pos, last_space_pos_stream;
			u32 go = BT_LINE_SIZE - 1;
			last_space_pos = last_space_pos_stream = 0;
			while (go && !gf_gzeof(parser->gz_in) ) {
				c1 = gf_gzgetc(parser->gz_in);
				c2 = gf_gzgetc(parser->gz_in);
				if (parser->unicode_type==2) {
					if (c2) {
						wchar = c2;
						wchar <<=8;
						wchar |= c1;
					}
					else wchar = c1;
				} else {
					wchar = c1;
					if (c2) {
						wchar <<= 8;
						wchar |= c2;
					}
				}
				*dst = wchar;
				if (wchar=='\r') is_ret = 1;
				else if (wchar=='\n') {
					dst++;
					break;
				}
				else if (is_ret) {
					u32 fpos = (u32) gf_gztell(parser->gz_in);
					gf_gzseek(parser->gz_in, fpos-2, SEEK_SET);
					break;
				}
				if (wchar==' ') {
					last_space_pos = (u32) (dst - l);
				}
				dst++;
				go--;
			}
			*dst = 0;
			if (!go) {
				u32 rew_pos = (u32)  (gf_gztell(parser->gz_in) - 2*(dst - &l[last_space_pos]) );
				gf_gzseek(parser->gz_in, rew_pos, SEEK_SET);
				l[last_space_pos+1] = 0;
			}
			if (l[0]==0xFFFF) {
				parser->done = 1;
				return;
			}
			dst = l;
			gf_utf8_wcstombs(parser->line_buffer, BT_LINE_SIZE, (const unsigned short **) &dst);
			if (!strlen(parser->line_buffer) && gf_gzeof(parser->gz_in)) {
				parser->done = 1;
				return;
			}
		} else {
			if ((gf_gzgets(parser->gz_in, parser->line_buffer, BT_LINE_SIZE) == NULL)
			        || (!strlen(parser->line_buffer) && gf_gzeof(parser->gz_in))) {
				parser->done = 1;
				return;
			}
			if (1 + strlen(parser->line_buffer) == BT_LINE_SIZE) {
				u32 rew, pos, go;
				rew = 0;
				go = 1;
				while (go) {
					switch (parser->line_buffer[strlen(parser->line_buffer)-1]) {
					case ' ':
					case ',':
					case '[':
					case ']':
						go = 0;
						break;
					default:
						parser->line_buffer[strlen(parser->line_buffer)-1] = 0;
						rew++;
						break;
					}
				}
				pos = (u32) gf_gztell(parser->gz_in);
				gf_gzseek(parser->gz_in, pos-rew, SEEK_SET);
			}
		}
		while (1) {
			char c;
			u32 len = (u32) strlen(parser->line_buffer);
			if (!len) break;
			c = parser->line_buffer[len-1];
			if (!strchr("\n\r\t", c)) break;
			parser->line_buffer[len-1] = 0;
		}
		parser->line_size = (u32) strlen(parser->line_buffer);
		parser->line_pos = 0;
		parser->line++;
		{
			u32 pos = (u32) gf_gztell(parser->gz_in);
			if (pos>=parser->file_pos) {
				parser->file_pos = pos;
				if (parser->line>1) gf_set_progress("BT Parsing", pos, parser->file_size);
			}
		}
		while ((parser->line_buffer[parser->line_pos]==' ') || (parser->line_buffer[parser->line_pos]=='\t'))
			parser->line_pos++;
		if ( (parser->line_buffer[parser->line_pos]=='#')
		        || ( (parser->line_buffer[parser->line_pos]=='/') && (parser->line_buffer[parser->line_pos+1]=='/')) ) {
			if (parser->line==1) {
				if (strstr(parser->line_buffer, "VRML")) {
					if (strstr(parser->line_buffer, "VRML V2.0")) parser->is_wrl = 1;
					else if (strstr(parser->line_buffer, "VRML2.0")) parser->is_wrl = 1;
					else {
						gf_bt_report(parser, GF_NOT_SUPPORTED, "%s: VRML Version Not Supported", parser->line_buffer);
						return;
					}
				}
				else if (strstr(parser->line_buffer, "X3D")) {
					if (strstr(parser->line_buffer, "X3D V3.0")) parser->is_wrl = 2;
					else {
						gf_bt_report(parser, GF_NOT_SUPPORTED, "%s: X3D Version Not Supported", parser->line_buffer);
						return;
					}
				}
			}
			if (!strnicmp(parser->line_buffer+parser->line_pos, "#define ", 8) && !parser->block_comment) {
				char *buf, *sep;
				parser->line_pos+=8;
				buf = parser->line_buffer+parser->line_pos;
				sep = strchr(buf, ' ');
				if (sep && (sep[1]!='\n') ) {
					BTDefSymbol *def;
					GF_SAFEALLOC(def, BTDefSymbol);
					if (!def) {
						GF_LOG(GF_LOG_ERROR, GF_LOG_PARSER, ("Fail to allocate DEF node\n"));
						return;
					}
					sep[0] = 0;
					def->name = gf_strdup(buf);
					sep[0] = ' ';
					buf = sep+1;
					while (strchr(" \t", buf[0])) buf++;
					def->value = gf_strdup(buf);
					gf_list_add(parser->def_symbols, def);
				}
			}
			else if (!strnicmp(parser->line_buffer+parser->line_pos, "#if ", 4)) {
				u32 len = 0;
				parser->line_pos+=4;
				while (1) {
					if (parser->line_pos+(s32)len==parser->line_size) break;
					if (strchr(" \n\t", parser->line_buffer[parser->line_pos+len]))
						break;
					len++;
				}
				if (len) {
					if (len==1) {
						if (!strnicmp(parser->line_buffer+parser->line_pos, "0", 1)) {
							parser->block_comment++;
						}
					} else {
						u32 i, count;
						char *keyWord = NULL;
						count = gf_list_count(parser->def_symbols);
						for (i=0; i<count; i++) {
							BTDefSymbol *def = (BTDefSymbol *)gf_list_get(parser->def_symbols, i);
							if (!strnicmp(parser->line_buffer+parser->line_pos, def->name, len)) {
								keyWord = def->value;
								break;
							}
						}
						if (keyWord && !strcmp(keyWord, "0")) {
							parser->block_comment++;
						}
					}
				}
			}
			else if (!strnicmp(parser->line_buffer+parser->line_pos, "#endif", 6)) {
				if (parser->block_comment) parser->block_comment--;
			}
			else if (!strnicmp(parser->line_buffer+parser->line_pos, "#else", 5)) {
				if (parser->block_comment)
					parser->block_comment--;
				else
					parser->block_comment++;
			}
			else if (!strnicmp(parser->line_buffer+parser->line_pos, "#size", 5)) {
				char *buf;
				parser->line_pos+=6;
				buf = parser->line_buffer+parser->line_pos;
				while (strchr(" \t", buf[0]))
					buf++;
				sscanf(buf, "%dx%d", &parser->def_w, &parser->def_h);
			}
			goto next_line;
		}
		if (parser->block_comment)
			goto next_line;
		if (parser->line_pos < parser->line_size) {
			u32 i, count;
			count = gf_list_count(parser->def_symbols);
			while (1) {
				Bool found = 0;
				for (i=0; i<count; i++) {
					u32 symb_len, val_len, copy_len;
					BTDefSymbol *def = (BTDefSymbol *)gf_list_get(parser->def_symbols, i);
					char *start = strstr(parser->line_buffer, def->name);
					if (!start) continue;
					symb_len = (u32) strlen(def->name);
					if (!strchr(" \n\r\t,[]{}\'\"", start[symb_len])) continue;
					val_len = (u32) strlen(def->value);
					copy_len = (u32) strlen(start + symb_len) + 1;
					memmove(start + val_len, start + symb_len, sizeof(char)*copy_len);
					memcpy(start, def->value, sizeof(char)*val_len);
					parser->line_size = (u32) strlen(parser->line_buffer);
					found = 1;
				}
				if (!found) break;
			}
		}
	}
	if (!parser->line_size) {
		if (!gf_gzeof(parser->gz_in)) gf_bt_check_line(parser);
		else parser->done = 1;
	}
	else if (!parser->done && (parser->line_size == parser->line_pos)) gf_bt_check_line(parser);
}