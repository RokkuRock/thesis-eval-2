char *gf_bt_get_next(GF_BTParser *parser, Bool point_break)
{
	u32 has_quote;
	Bool go = 1;
	s32 i;
	gf_bt_check_line(parser);
	i=0;
	has_quote = 0;
	while (go) {
		if (parser->line_buffer[parser->line_pos + i] == '\"') {
			if (!has_quote) has_quote = 1;
			else has_quote = 0;
			parser->line_pos += 1;
			if (parser->line_pos+i==parser->line_size) break;
			continue;
		}
		if (!has_quote) {
			switch (parser->line_buffer[parser->line_pos + i]) {
			case 0:
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '{':
			case '}':
			case ']':
			case '[':
			case ',':
				go = 0;
				break;
			case '.':
				if (point_break) go = 0;
				break;
			}
			if (!go) break;
		}
		parser->cur_buffer[i] = parser->line_buffer[parser->line_pos + i];
		i++;
		if (parser->line_pos+i==parser->line_size) break;
	}
	parser->cur_buffer[i] = 0;
	parser->line_pos += i;
	return parser->cur_buffer;
}