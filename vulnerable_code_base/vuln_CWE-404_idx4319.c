GF_Err gf_svg_parse_attribute(GF_Node *n, GF_FieldInfo *info, char *attribute_content, u8 anim_value_type)
{
	GF_Err e = GF_OK;
	if (info->fieldType != DOM_String_datatype && strlen(attribute_content)) {
		u32 i, len;
		while (attribute_content[0] && (strchr("\r\n\t ", attribute_content[0])))
			attribute_content++;
		i=0;
		len = (u32) strlen(attribute_content);
		while (i<len) {
			if (strchr("\r\n\t", attribute_content[i]))
				attribute_content[i] = ' ';
			i++;
		}
		while (len && attribute_content[len-1]==' ') {
			attribute_content[len-1] = 0;
			len--;
		}
	}
	switch (info->fieldType) {
	case SVG_Boolean_datatype:
		svg_parse_boolean((SVG_Boolean *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Color_datatype:
		svg_parse_color((SVG_Color *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Paint_datatype:
		svg_parse_paint(n, (SVG_Paint *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_FillRule_datatype:
		svg_parse_clipfillrule((SVG_FillRule *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_StrokeLineJoin_datatype:
		svg_parse_strokelinejoin((SVG_StrokeLineJoin *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_StrokeLineCap_datatype:
		svg_parse_strokelinecap((SVG_StrokeLineCap *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_FontStyle_datatype:
		svg_parse_fontstyle((SVG_FontStyle *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_FontWeight_datatype:
		svg_parse_fontweight((SVG_FontWeight *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_FontVariant_datatype:
		svg_parse_fontvariant((SVG_FontVariant *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_TextAnchor_datatype:
		svg_parse_textanchor((SVG_TextAnchor *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Display_datatype:
		svg_parse_display((SVG_Display *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Visibility_datatype:
		svg_parse_visibility((SVG_Visibility *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Overflow_datatype:
		svg_parse_overflow((SVG_Overflow *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_ZoomAndPan_datatype:
		svg_parse_zoomandpan((SVG_ZoomAndPan *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_DisplayAlign_datatype:
		svg_parse_displayalign((SVG_DisplayAlign *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_TextAlign_datatype:
		svg_parse_textalign((SVG_TextAlign *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_PointerEvents_datatype:
		svg_parse_pointerevents((SVG_PointerEvents *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_RenderingHint_datatype:
		svg_parse_renderinghint((SVG_RenderingHint *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_VectorEffect_datatype:
		svg_parse_vectoreffect((SVG_VectorEffect *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_PlaybackOrder_datatype:
		svg_parse_playbackorder((SVG_PlaybackOrder *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_TimelineBegin_datatype:
		svg_parse_timelinebegin((SVG_TimelineBegin *)info->far_ptr, attribute_content, &e);
		break;
	case XML_Space_datatype:
		svg_parse_xmlspace((XML_Space *)info->far_ptr, attribute_content, &e);
		break;
	case XMLEV_Propagate_datatype:
		svg_parse_xmlev_propagate((XMLEV_Propagate *)info->far_ptr, attribute_content, &e);
		break;
	case XMLEV_DefaultAction_datatype:
		svg_parse_xmlev_defaultAction((XMLEV_DefaultAction *)info->far_ptr, attribute_content, &e);
		break;
	case XMLEV_Phase_datatype:
		svg_parse_xmlev_phase((XMLEV_Phase *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_SyncBehavior_datatype:
		smil_parse_syncBehaviorOrDefault((SMIL_SyncBehavior *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_SyncTolerance_datatype:
		smil_parse_syncToleranceOrDefault((SMIL_SyncTolerance *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_AttributeType_datatype:
		smil_parse_attributeType((SMIL_AttributeType *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_CalcMode_datatype:
		smil_parse_calcmode((SMIL_CalcMode *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_Additive_datatype:
		smil_parse_additive((SMIL_CalcMode *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_Accumulate_datatype:
		smil_parse_accumulate((SMIL_Accumulate *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_Restart_datatype:
		smil_parse_restart((SMIL_Restart *)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_Fill_datatype:
		smil_parse_fill((SMIL_Fill *)info->far_ptr, attribute_content, &e);
		break;
	case SVG_GradientUnit_datatype:
		if (!strcmp(attribute_content, "userSpaceOnUse"))
			*((SVG_GradientUnit *)info->far_ptr) = SVG_GRADIENTUNITS_USER;
		else if (!strcmp(attribute_content, "objectBoundingBox"))
			*((SVG_GradientUnit *)info->far_ptr) = SVG_GRADIENTUNITS_OBJECT;
		else
			e = GF_NON_COMPLIANT_BITSTREAM;
		break;
	case SVG_FocusHighlight_datatype:
		svg_parse_focushighlight((SVG_FocusHighlight*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Focusable_datatype:
		svg_parse_focusable((SVG_Focusable*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_InitialVisibility_datatype:
		svg_parse_initialvisibility((SVG_InitialVisibility*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Overlay_datatype:
		svg_parse_overlay((SVG_Overlay*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_TransformBehavior_datatype:
		svg_parse_transformbehavior((SVG_TransformBehavior*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_SpreadMethod_datatype:
		if (!strcmp(attribute_content, "reflect")) *(u8*)info->far_ptr = SVG_SPREAD_REFLECT;
		else if (!strcmp(attribute_content, "repeat")) *(u8*)info->far_ptr = SVG_SPREAD_REPEAT;
		else if (!strcmp(attribute_content, "pad")) *(u8*)info->far_ptr = SVG_SPREAD_PAD;
		else e = GF_NON_COMPLIANT_BITSTREAM;
		break;
	case SVG_Filter_TransferType_datatype:
		if (!strcmp(attribute_content, "table")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_TABLE;
		else if (!strcmp(attribute_content, "discrete")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_DISCRETE;
		else if (!strcmp(attribute_content, "linear")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_LINEAR;
		else if (!strcmp(attribute_content, "gamma")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_GAMMA;
		else if (!strcmp(attribute_content, "identity")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_IDENTITY;
		else if (!strcmp(attribute_content, "fractalNoise")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_FRACTAL_NOISE;
		else if (!strcmp(attribute_content, "turbulence")) *(u8*)info->far_ptr = SVG_FILTER_TRANSFER_TURBULENCE;
		else if (!strcmp(attribute_content, "matrix")) *(u8*)info->far_ptr = SVG_FILTER_MX_MATRIX;
		else if (!strcmp(attribute_content, "saturate")) *(u8*)info->far_ptr = SVG_FILTER_MX_SATURATE;
		else if (!strcmp(attribute_content, "hueRotate")) *(u8*)info->far_ptr = SVG_FILTER_HUE_ROTATE;
		else if (!strcmp(attribute_content, "luminanceToAlpha")) *(u8*)info->far_ptr = SVG_FILTER_LUM_TO_ALPHA;
		else e = GF_NON_COMPLIANT_BITSTREAM;
		break;
	case SVG_Length_datatype:
	case SVG_Coordinate_datatype:
	case SVG_FontSize_datatype:
	case SVG_Rotate_datatype:
	case SVG_Number_datatype:
		svg_parse_length((SVG_Number*)info->far_ptr, attribute_content, 0, &e);
		break;
	case SMIL_AnimateValue_datatype:
		svg_parse_one_anim_value(n, (SMIL_AnimateValue*)info->far_ptr, attribute_content, anim_value_type, &e);
		break;
	case SMIL_AnimateValues_datatype:
		svg_parse_anim_values(n, (SMIL_AnimateValues*)info->far_ptr, attribute_content, anim_value_type, &e);
		break;
	case XMLRI_datatype:
		svg_parse_iri(n, (XMLRI*)info->far_ptr, attribute_content);
		break;
	case XML_IDREF_datatype:
		svg_parse_idref(n, (XMLRI*)info->far_ptr, attribute_content);
		break;
	case SMIL_AttributeName_datatype:
		((SMIL_AttributeName *)info->far_ptr)->name = gf_strdup(attribute_content);
		break;
	case SMIL_Times_datatype:
		smil_parse_time_list(n, *(GF_List **)info->far_ptr, attribute_content);
		break;
	case SMIL_Duration_datatype:
		smil_parse_min_max_dur_repeatdur((SMIL_Duration*)info->far_ptr, attribute_content, &e);
		break;
	case SMIL_RepeatCount_datatype:
		smil_parse_repeatcount((SMIL_RepeatCount*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_PathData_datatype:
		svg_parse_path((SVG_PathData*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Points_datatype:
		svg_parse_points(*(GF_List **)(info->far_ptr), attribute_content, &e);
		break;
	case SMIL_KeyTimes_datatype:
	case SMIL_KeyPoints_datatype:
	case SMIL_KeySplines_datatype:
	case SVG_Numbers_datatype:
		svg_parse_numbers(*(GF_List **)(info->far_ptr), attribute_content, 0, &e);
		break;
	case SVG_Coordinates_datatype:
		svg_parse_coordinates(*(GF_List **)(info->far_ptr), attribute_content, &e);
		break;
	case SVG_ViewBox_datatype:
		svg_parse_viewbox((SVG_ViewBox*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_StrokeDashArray_datatype:
		svg_parse_strokedasharray((SVG_StrokeDashArray*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_FontFamily_datatype:
		svg_parse_fontfamily((SVG_FontFamily*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Motion_datatype:
		svg_parse_point_into_matrix((GF_Matrix2D*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Transform_datatype:
		e = svg_parse_transform((SVG_Transform*)info->far_ptr, attribute_content);
		break;
	case SVG_Transform_Translate_datatype:
	{
		u32 i = 0;
		SVG_Point *p = (SVG_Point *)info->far_ptr;
		i+=svg_parse_number(&(attribute_content[i]), &(p->x), 0, &e);
		if (attribute_content[i] == 0) {
			p->y = 0;
		} else {
			 svg_parse_number(&(attribute_content[i]), &(p->y), 0, &e);
		}
	}
	break;
	case SVG_Transform_Scale_datatype:
	{
		u32 i = 0;
		SVG_Point *p = (SVG_Point *)info->far_ptr;
		i+=svg_parse_number(&(attribute_content[i]), &(p->x), 0, &e);
		if (attribute_content[i] == 0) {
			p->y = p->x;
		} else {
			 svg_parse_number(&(attribute_content[i]), &(p->y), 0, &e);
		}
	}
	break;
	case SVG_Transform_SkewX_datatype:
	case SVG_Transform_SkewY_datatype:
	{
		Fixed *p = (Fixed *)info->far_ptr;
		svg_parse_number(attribute_content, p, 1, &e);
	}
	break;
	case SVG_Transform_Rotate_datatype:
	{
		u32 i = 0;
		SVG_Point_Angle *p = (SVG_Point_Angle *)info->far_ptr;
		i+=svg_parse_number(&(attribute_content[i]), &(p->angle), 1, &e);
		if (attribute_content[i] == 0) {
			p->y = p->x = 0;
		} else {
			i+=svg_parse_number(&(attribute_content[i]), &(p->x), 0, &e);
			 svg_parse_number(&(attribute_content[i]), &(p->y), 0, &e);
		}
	}
	break;
	case SVG_PreserveAspectRatio_datatype:
		svg_parse_preserveaspectratio((SVG_PreserveAspectRatio*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_TransformType_datatype:
		svg_parse_animatetransform_type((SVG_TransformType*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_ID_datatype:
	case DOM_String_datatype:
	case SVG_ContentType_datatype:
	case SVG_LanguageID_datatype:
		if (*(SVG_String *)info->far_ptr) gf_free(*(SVG_String *)info->far_ptr);
		*(SVG_String *)info->far_ptr = gf_strdup(attribute_content);
		break;
	case DOM_StringList_datatype:
		svg_parse_strings(*(GF_List **)info->far_ptr, attribute_content, 0);
		break;
	case XMLRI_List_datatype:
		svg_parse_strings(*(GF_List **)info->far_ptr, attribute_content, 1);
		break;
	case XMLEV_Event_datatype:
	{
		XMLEV_Event *xml_ev = (XMLEV_Event *)info->far_ptr;
		char *sep = strchr(attribute_content, '(');
		if (sep) {
			sep[0] = 0;
			xml_ev->type = gf_dom_event_type_by_name(attribute_content);
			sep[0] = '(';
			if ((xml_ev->type == GF_EVENT_REPEAT) || (xml_ev->type == GF_EVENT_REPEAT_EVENT)) {
				char _v;
				sscanf(sep, "(%c)", &_v);
				xml_ev->parameter = _v;
			} else {  
				char *sep2 = strchr(attribute_content, ')');
				sep2[0] = 0;
				xml_ev->parameter = gf_dom_get_key_type(sep+1);
				sep2[0] = ')';
			}
		} else {
			xml_ev->parameter = 0;
			xml_ev->type = gf_dom_event_type_by_name(attribute_content);
		}
	}
	break;
	case SVG_Focus_datatype:
		svg_parse_focus(n, (SVG_Focus*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_ClipPath_datatype:
		svg_parse_clippath(n, (SVG_ClipPath*)info->far_ptr, attribute_content, &e);
		break;
	case LASeR_Choice_datatype:
		e = laser_parse_choice((LASeR_Choice*)info->far_ptr, attribute_content);
		break;
	case LASeR_Size_datatype:
		e = laser_parse_size((LASeR_Size*)info->far_ptr, attribute_content, &e);
		break;
	case SVG_Clock_datatype:
		svg_parse_clock_value(attribute_content, (SVG_Clock*)info->far_ptr);
		break;
	case SVG_Unknown_datatype:
		if (*(SVG_String *)info->far_ptr) gf_free(*(SVG_String *)info->far_ptr);
		*(SVG_String *)info->far_ptr = gf_strdup(attribute_content);
		break;
	default:
		GF_LOG(GF_LOG_WARNING, GF_LOG_PARSER, ("[SVG Parsing] Cannot parse attribute %s\n", info->name ? info->name : ""));
		break;
	}
	if (e) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_PARSER, ("[SVG Parsing] Cannot parse attribute %s value %s: %s\n", info->name ? info->name : "", attribute_content, gf_error_to_string(e)));
	}
	return e;
}