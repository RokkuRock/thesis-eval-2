static GF_Err swf_def_font(SWFReader *read, u32 revision)
{
	u32 i, count;
	GF_Err e;
	SWFFont *ft;
	u32 *offset_table = NULL;
	u32 start;
	GF_SAFEALLOC(ft, SWFFont);
	if (!ft) return GF_OUT_OF_MEM;
	ft->glyphs = gf_list_new();
	ft->fontID = swf_get_16(read);
	e = GF_OK;
	if (revision==0) {
		start = swf_get_file_pos(read);
		count = swf_get_16(read);
		ft->nbGlyphs = count / 2;
		offset_table = (u32*)gf_malloc(sizeof(u32) * ft->nbGlyphs);
		offset_table[0] = 0;
		for (i=1; i<ft->nbGlyphs; i++) offset_table[i] = swf_get_16(read);
		for (i=0; i<ft->nbGlyphs; i++) {
			swf_align(read);
			e = swf_seek_file_to(read, start + offset_table[i]);
			if (e) break;
			swf_parse_shape_def(read, ft, 0);
		}
		gf_free(offset_table);
		if (e) return e;
	} else if (revision==1) {
		SWFRec rc;
		Bool wide_offset, wide_codes;
		u32 code_offset, checkpos;
		ft->has_layout = swf_read_int(read, 1);
		ft->has_shiftJIS = swf_read_int(read, 1);
		ft->is_unicode = swf_read_int(read, 1);
		ft->is_ansi = swf_read_int(read, 1);
		wide_offset = swf_read_int(read, 1);
		wide_codes = swf_read_int(read, 1);
		ft->is_italic = swf_read_int(read, 1);
		ft->is_bold = swf_read_int(read, 1);
		swf_read_int(read, 8);
		count = swf_read_int(read, 8);
		ft->fontName = (char*)gf_malloc(sizeof(u8)*count+1);
		ft->fontName[count] = 0;
		for (i=0; i<count; i++) ft->fontName[i] = swf_read_int(read, 8);
		ft->nbGlyphs = swf_get_16(read);
		start = swf_get_file_pos(read);
		if (ft->nbGlyphs) {
			offset_table = (u32*)gf_malloc(sizeof(u32) * ft->nbGlyphs);
			for (i=0; i<ft->nbGlyphs; i++) {
				if (wide_offset) offset_table[i] = swf_get_32(read);
				else offset_table[i] = swf_get_16(read);
			}
		}
		if (wide_offset) {
			code_offset = swf_get_32(read);
		} else {
			code_offset = swf_get_16(read);
		}
		if (ft->nbGlyphs) {
			for (i=0; i<ft->nbGlyphs; i++) {
				swf_align(read);
				e = swf_seek_file_to(read, start + offset_table[i]);
				if (e) break;
				swf_parse_shape_def(read, ft, 0);
			}
			gf_free(offset_table);
			if (e) return e;
			checkpos = swf_get_file_pos(read);
			if (checkpos != start + code_offset) {
				GF_LOG(GF_LOG_ERROR, GF_LOG_PARSER, ("[SWF Parsing] bad code offset in font\n"));
				return GF_NON_COMPLIANT_BITSTREAM;
			}
			ft->glyph_codes = (u16*)gf_malloc(sizeof(u16) * ft->nbGlyphs);
			for (i=0; i<ft->nbGlyphs; i++) {
				if (wide_codes) ft->glyph_codes[i] = swf_get_16(read);
				else ft->glyph_codes[i] = swf_read_int(read, 8);
			}
		}
		if (ft->has_layout) {
			ft->ascent = swf_get_s16(read);
			ft->descent = swf_get_s16(read);
			ft->leading = swf_get_s16(read);
			if (ft->nbGlyphs) {
				ft->glyph_adv = (s16*)gf_malloc(sizeof(s16) * ft->nbGlyphs);
				for (i=0; i<ft->nbGlyphs; i++) ft->glyph_adv[i] = swf_get_s16(read);
				for (i=0; i<ft->nbGlyphs; i++) swf_get_rec(read, &rc);
			}
			count = swf_get_16(read);
			for (i=0; i<count; i++) {
				if (wide_codes) {
					swf_get_16(read);
					swf_get_16(read);
				} else {
					swf_read_int(read, 8);
					swf_read_int(read, 8);
				}
				swf_get_s16(read);
			}
		}
	}
	gf_list_add(read->fonts, ft);
	return GF_OK;
}