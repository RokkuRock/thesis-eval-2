static void lsr_read_rare_full(GF_LASeRCodec *lsr, GF_Node *n)
{
	GF_FieldInfo info;
	u32 i, nb_rare, field_rare;
	s32 field_tag;
	GF_LSR_READ_INT(lsr, nb_rare, 1, "has_rare");
	if (!nb_rare) return;
	GF_LSR_READ_INT(lsr, nb_rare, 6, "nbOfAttributes");
	for (i=0; i<nb_rare; i++) {
		GF_LSR_READ_INT(lsr, field_rare, 6, "attributeRARE");
		if (field_rare==49) {
			u32 extID, len, j;
			while (1) {
				GF_LSR_READ_INT(lsr, extID, lsr->info->cfg.extensionIDBits, "extensionID");
				len = lsr_read_vluimsbf5(lsr, "len");
				if (extID==2) {
					GF_LSR_READ_INT(lsr, len, 2, "nbOfAttributes");
					for (j=0; j<len; j++) {
						GF_LSR_READ_INT(lsr, extID, 3, "attributeRARE");
						switch (extID) {
						case 0:
							lsr->last_error = gf_node_get_attribute_by_tag(n, TAG_SVG_ATT_syncMaster, GF_TRUE, GF_FALSE, &info);
							GF_LSR_READ_INT(lsr, *(SVG_Boolean *)info.far_ptr, 1, "syncMaster");
							break;
						case 1:
							lsr->last_error = gf_node_get_attribute_by_tag(n, TAG_SVG_ATT_focusHighlight, GF_TRUE, GF_FALSE, &info);
							GF_LSR_READ_INT(lsr, *(SVG_FocusHighlight *)info.far_ptr, 2, "focusHighlight");
							break;
						case 2:
							lsr->last_error = gf_node_get_attribute_by_tag(n, TAG_SVG_ATT_initialVisibility, GF_TRUE, GF_FALSE, &info);
							GF_LSR_READ_INT(lsr, *(SVG_InitialVisibility *)info.far_ptr, 2, "initialVisibility");
							break;
						case 3:
							lsr->last_error = gf_node_get_attribute_by_tag(n, TAG_SVG_ATT_fullscreen, GF_TRUE, GF_FALSE, &info);
							GF_LSR_READ_INT(lsr, *(SVG_Boolean *)info.far_ptr, 1, "fullscreen");
							break;
						case 4:
							lsr->last_error = gf_node_get_attribute_by_tag(n, TAG_SVG_ATT_requiredFonts, GF_TRUE, GF_FALSE, &info);
							lsr_read_byte_align_string_list(lsr, *(GF_List **)info.far_ptr, "requiredFonts", GF_FALSE, GF_TRUE);
							break;
						}
					}
				} else {
					gf_bs_read_int(lsr->bs, len);
				}
				GF_LSR_READ_INT(lsr, extID, 1, "hasNextExtension");
				if (!extID) break;
			}
			continue;
		}
		field_tag = gf_lsr_rare_type_to_attribute(field_rare);
		if (field_tag==-1) {
			return;
		}
		lsr->last_error = gf_node_get_attribute_by_tag(n, field_tag, GF_TRUE, GF_FALSE, &info);
		if (!info.far_ptr) lsr->last_error = GF_NOT_SUPPORTED;
		if (lsr->last_error) return;
		switch (field_tag) {
		case TAG_SVG_ATT__class:
			lsr_read_byte_align_string(lsr, info.far_ptr, "class");
			break;
		case TAG_SVG_ATT_audio_level:
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "audio-level");
			break;
		case TAG_SVG_ATT_color:
			lsr_read_paint(lsr, (SVG_Paint *)info.far_ptr, "color");
			break;
		case TAG_SVG_ATT_color_rendering:
			GF_LSR_READ_INT(lsr, *(SVG_RenderingHint*)info.far_ptr, 2, "color-rendering");
			break;
		case TAG_SVG_ATT_display:
			GF_LSR_READ_INT(lsr, *(SVG_Display*)info.far_ptr, 5, "display");
			break;
		case TAG_SVG_ATT_display_align:
			GF_LSR_READ_INT(lsr, *(SVG_DisplayAlign*)info.far_ptr, 3, "display-align");
			break;
		case TAG_SVG_ATT_fill_opacity:
			((SVG_Number*)info.far_ptr)->type = SVG_NUMBER_VALUE;
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "fill-opacity");
			break;
		case TAG_SVG_ATT_fill_rule:
			GF_LSR_READ_INT(lsr, *(SVG_FillRule*)info.far_ptr, 2, "fill-rule");
			break;
		case TAG_SVG_ATT_image_rendering:
			GF_LSR_READ_INT(lsr, *(SVG_RenderingHint*)info.far_ptr, 2, "image-rendering");
			break;
		case TAG_SVG_ATT_line_increment:
			lsr_read_line_increment_type(lsr, info.far_ptr, "line-increment");
			break;
		case TAG_SVG_ATT_pointer_events:
			GF_LSR_READ_INT(lsr, *(SVG_PointerEvents*)info.far_ptr, 4, "pointer-events");
			break;
		case TAG_SVG_ATT_shape_rendering:
			GF_LSR_READ_INT(lsr, *(SVG_RenderingHint*)info.far_ptr, 3, "shape-rendering");
			break;
		case TAG_SVG_ATT_solid_color:
			lsr_read_paint(lsr, info.far_ptr, "solid-color");
			break;
		case TAG_SVG_ATT_solid_opacity:
			((SVG_Number*)info.far_ptr)->type = SVG_NUMBER_VALUE;
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "solid-opacity");
			break;
		case TAG_SVG_ATT_stop_color:
			lsr_read_paint(lsr, info.far_ptr, "stop-color");
			break;
		case TAG_SVG_ATT_stop_opacity:
			((SVG_Number*)info.far_ptr)->type = SVG_NUMBER_VALUE;
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "stop-opacity");
			break;
		case TAG_SVG_ATT_stroke_dasharray:
		{
			u32 j, flag;
			SVG_StrokeDashArray *da = (SVG_StrokeDashArray *)info.far_ptr;
			GF_LSR_READ_INT(lsr, flag, 1, "dashArray");
			if (flag) {
				da->type=SVG_STROKEDASHARRAY_INHERIT;
			} else {
				da->type=SVG_STROKEDASHARRAY_ARRAY;
				da->array.count = lsr_read_vluimsbf5(lsr, "len");
				da->array.vals = (Fixed*)gf_malloc(sizeof(Fixed)*da->array.count);
				da->array.units = (u8*)gf_malloc(sizeof(u8)*da->array.count);
				if (!da->array.vals || !da->array.units) {
					lsr->last_error = GF_OUT_OF_MEM;
					return;
				}
				for (j=0; j<da->array.count; j++) {
					da->array.vals[j] = lsr_read_fixed_16_8(lsr, "dash");
					da->array.units[j] = 0;
					if (lsr->last_error) return;
				}
			}
		}
		break;
		case TAG_SVG_ATT_stroke_dashoffset:
			lsr_read_fixed_16_8i(lsr, info.far_ptr, "dashOffset");
			break;
		case TAG_SVG_ATT_stroke_linecap:
			GF_LSR_READ_INT(lsr, *(SVG_StrokeLineCap*)info.far_ptr, 2, "stroke-linecap");
			break;
		case TAG_SVG_ATT_stroke_linejoin:
			GF_LSR_READ_INT(lsr, *(SVG_StrokeLineJoin*)info.far_ptr, 2, "stroke-linejoin");
			break;
		case TAG_SVG_ATT_stroke_miterlimit:
			lsr_read_fixed_16_8i(lsr, info.far_ptr, "miterLimit");
			break;
		case TAG_SVG_ATT_stroke_opacity:
			((SVG_Number*)info.far_ptr)->type = SVG_NUMBER_VALUE;
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "stroke-opacity");
			break;
		case TAG_SVG_ATT_stroke_width:
			lsr_read_fixed_16_8i(lsr, info.far_ptr, "strokeWidth");
			break;
		case TAG_SVG_ATT_text_anchor:
			GF_LSR_READ_INT(lsr, *(SVG_TextAnchor*)info.far_ptr, 2, "text-achor");
			break;
		case TAG_SVG_ATT_text_rendering:
			GF_LSR_READ_INT(lsr, *(SVG_RenderingHint*)info.far_ptr, 3, "text-rendering");
			break;
		case TAG_SVG_ATT_viewport_fill:
			lsr_read_paint(lsr, info.far_ptr, "viewport-fill");
			break;
		case TAG_SVG_ATT_viewport_fill_opacity:
			((SVG_Number*)info.far_ptr)->type = SVG_NUMBER_VALUE;
			((SVG_Number*)info.far_ptr)->value = lsr_read_fixed_clamp(lsr, "viewport-fill-opacity");
			break;
		case TAG_SVG_ATT_vector_effect:
			GF_LSR_READ_INT(lsr, *(SVG_VectorEffect*)info.far_ptr, 4, "vector-effect");
			break;
		case TAG_SVG_ATT_visibility:
			GF_LSR_READ_INT(lsr, *(SVG_Visibility*)info.far_ptr, 2, "visibility");
			break;
		case TAG_SVG_ATT_requiredExtensions:
			lsr_read_byte_align_string_list(lsr, *(GF_List**)info.far_ptr, "requiredExtensions", GF_TRUE, GF_FALSE);
			break;
		case TAG_SVG_ATT_requiredFormats:
			lsr_read_byte_align_string_list(lsr, *(GF_List**)info.far_ptr, "requiredFormats", GF_FALSE, GF_FALSE);
			break;
		case TAG_SVG_ATT_requiredFeatures:
		{
			u32 j, fcount = lsr_read_vluimsbf5(lsr, "count");
			for (j=0; j<fcount; j++) {
				u32 fval;
				GF_LSR_READ_INT(lsr, fval, 6, "feature");
				if (lsr->last_error) return;
			}
		}
		break;
		case TAG_SVG_ATT_systemLanguage:
			lsr_read_byte_align_string_list(lsr, *(GF_List**)info.far_ptr, "systemLanguage", GF_FALSE, GF_FALSE);
			break;
		case TAG_XML_ATT_base:
			lsr_read_byte_align_string(lsr, &((XMLRI*)info.far_ptr)->string, "xml:base");
			((XMLRI*)info.far_ptr)->type = XMLRI_STRING;
			break;
		case TAG_XML_ATT_lang:
			lsr_read_byte_align_string(lsr, info.far_ptr, "xml:lang");
			break;
		case TAG_XML_ATT_space:
			GF_LSR_READ_INT(lsr, *(XML_Space*)info.far_ptr, 1, "xml:space");
			break;
		case TAG_SVG_ATT_nav_next:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusNext");
			break;
		case TAG_SVG_ATT_nav_up:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusNorth");
			break;
		case TAG_SVG_ATT_nav_up_left:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusNorthEast");
			break;
		case TAG_SVG_ATT_nav_up_right:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusNorthWest");
			break;
		case TAG_SVG_ATT_nav_prev:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusPrev");
			break;
		case TAG_SVG_ATT_nav_down:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusSouth");
			break;
		case TAG_SVG_ATT_nav_down_left:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusSouthEast");
			break;
		case TAG_SVG_ATT_nav_down_right:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusSouthWest");
			break;
		case TAG_SVG_ATT_nav_left:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusEast");
			break;
		case TAG_SVG_ATT_focusable:
			GF_LSR_READ_INT(lsr, *(SVG_Focusable*)info.far_ptr, 2, "focusable");
			break;
		case TAG_SVG_ATT_nav_right:
			lsr_read_focus(lsr, (SVG_Focus*)info.far_ptr, "focusWest");
			break;
		case TAG_SVG_ATT_transform:
			lsr_read_matrix(lsr, info.far_ptr);
			break;
		case TAG_SVG_ATT_text_decoration:
			lsr_read_byte_align_string_list(lsr, *(GF_List**)info.far_ptr, "textDecoration", GF_FALSE, GF_FALSE);
			break;
		case TAG_SVG_ATT_font_variant:
			GF_LSR_READ_INT(lsr, *(SVG_FontVariant*)info.far_ptr, 2, "font-variant");
			break;
		case TAG_SVG_ATT_font_family:
		{
			u32 flag;
			GF_LSR_READ_INT(lsr, flag, 1, "isInherit");
			if (flag) {
				((SVG_FontFamily*)info.far_ptr)->type = SVG_FONTFAMILY_INHERIT;
			} else {
				char *ft;
				((SVG_FontFamily*)info.far_ptr)->type = SVG_FONTFAMILY_VALUE;
				GF_LSR_READ_INT(lsr, flag, lsr->fontIndexBits, "fontIndex");
				ft = (char*)gf_list_get(lsr->font_table, flag);
				if (ft) ((SVG_FontFamily*)info.far_ptr)->value = gf_strdup(ft);
			}
		}
		break;
		case TAG_SVG_ATT_font_size:
			lsr_read_fixed_16_8i(lsr, info.far_ptr, "fontSize");
			break;
		case TAG_SVG_ATT_font_style:
			GF_LSR_READ_INT(lsr, *(SVG_FontStyle*)info.far_ptr, 3, "fontStyle");
			break;
		case TAG_SVG_ATT_font_weight:
			GF_LSR_READ_INT(lsr, *(SVG_FontWeight*)info.far_ptr, 4, "fontWeight");
			break;
		case TAG_XLINK_ATT_title:
			lsr_read_byte_align_string(lsr, info.far_ptr, "xlink:title");
			break;
		case TAG_XLINK_ATT_type:
			GF_LSR_READ_INT(lsr, field_rare, 3, "xlink:type");
			break;
		case TAG_XLINK_ATT_role:
			lsr_read_any_uri(lsr, info.far_ptr, "xlink:role");
			break;
		case TAG_XLINK_ATT_arcrole:
			lsr_read_any_uri(lsr, info.far_ptr, "xlink:arcrole");
			break;
		case TAG_XLINK_ATT_actuate:
			GF_LSR_READ_INT(lsr, field_rare, 2, "xlink:actuate");
			break;
		case TAG_XLINK_ATT_show:
			GF_LSR_READ_INT(lsr, field_rare, 3, "xlink:show");
			break;
		case TAG_SVG_ATT_end:
			lsr_read_smil_times(lsr, NULL, 0, info.far_ptr, "end", 0);
			break;
		case TAG_SVG_ATT_max:
			lsr_read_duration_ex(lsr, NULL, 0, info.far_ptr, "min", 0);
			break;
		case TAG_SVG_ATT_min:
			lsr_read_duration_ex(lsr, NULL, 0, info.far_ptr, "min", 0);
			break;
		}
		if (lsr->last_error) return;
	}
}