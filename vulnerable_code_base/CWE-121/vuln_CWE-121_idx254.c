static GF_Err gf_xml_sax_parse_intern(GF_SAXParser *parser, char *current)
{
	u32 count;
	count = gf_list_count(parser->entities);
	while (count) {
		char *entityEnd;
		XML_Entity *ent;
		char *entityStart = strstr(current, "&");
		Bool needs_text;
		u32 line_num;
		if (parser->in_entity) {
			u32 len;
			char *name;
			entityEnd = strstr(current, ";");
			if (!entityEnd) return xml_sax_append_string(parser, current);
			entityStart = strrchr(parser->buffer, '&');
			entityEnd[0] = 0;
			len = (u32) strlen(entityStart) + (u32) strlen(current) + 1;
			name = (char*)gf_malloc(sizeof(char)*len);
			sprintf(name, "%s%s;", entityStart+1, current);
			ent = gf_xml_locate_entity(parser, name, &needs_text);
			gf_free(name);
			if (!ent && !needs_text) {
				xml_sax_append_string(parser, current);
				xml_sax_parse(parser, GF_TRUE);
				entityEnd[0] = ';';
				current = entityEnd;
				continue;
			}
			assert(ent);
			parser->line_size -= (u32) strlen(entityStart);
			entityStart[0] = 0;
			parser->in_entity = GF_FALSE;
			entityEnd[0] = ';';
			current = entityEnd+1;
		} else {
			if (!entityStart) break;
			ent = gf_xml_locate_entity(parser, entityStart+1, &needs_text);
			entityStart[0] = 0;
			xml_sax_append_string(parser, current);
			xml_sax_parse(parser, GF_TRUE);
			entityStart[0] = '&';
			if (!ent && !needs_text) {
				xml_sax_append_string(parser, "&");
				current = entityStart+1;
				continue;
			}
			if (!ent) {
				parser->in_entity = GF_TRUE;
				return xml_sax_append_string(parser, entityStart);
			}
			current = entityStart + ent->namelen + 2;
		}
		line_num = parser->line;
		xml_sax_append_string(parser, ent->value);
		xml_sax_parse(parser, GF_TRUE);
		parser->line = line_num;
	}
	xml_sax_append_string(parser, current);
	return xml_sax_parse(parser, GF_FALSE);
}