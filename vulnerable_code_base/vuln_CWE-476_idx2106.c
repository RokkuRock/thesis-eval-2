GF_Err gf_bt_loader_run_intern(GF_BTParser *parser, GF_Command *init_com, Bool initial_run)
{
	char *str;
	GF_Node *node, *vrml_root_node;
	Bool force_new_com;
	GF_Route *r;
	Bool has_id;
	char szDEFName[1000];
	vrml_root_node = NULL;
	has_id = 0;
	if (init_com)
		parser->in_com = 0 ;
	parser->cur_com = init_com;
	force_new_com = (parser->load->flags & GF_SM_LOAD_CONTEXT_READY) ? 1 : 0;
	if (parser->is_wrl && !parser->top_nodes) {
		if (initial_run ) {
#ifndef GPAC_DISABLE_X3D
			vrml_root_node = gf_node_new(parser->load->scene_graph, (parser->load->flags & GF_SM_LOAD_MPEG4_STRICT) ? TAG_MPEG4_Group : TAG_X3D_Group);
#else
			vrml_root_node = gf_node_new(parser->load->scene_graph, TAG_MPEG4_Group);
#endif
			gf_node_register(vrml_root_node, NULL);
			gf_node_init(vrml_root_node);
			gf_sg_set_root_node(parser->load->scene_graph, vrml_root_node);
		} else {
			vrml_root_node = gf_sg_get_root_node(parser->load->scene_graph);
		}
	}
	if (!parser->in_com)
		parser->stream_id = parser->load->force_es_id;
	while (!parser->last_error) {
		str = gf_bt_get_next(parser, 0);
		if (parser->done) break;
		if (!strcmp(str, "PROFILE")) gf_bt_force_line(parser);
		else if (!strcmp(str, "COMPONENT")) gf_bt_force_line(parser);
		else if (!strcmp(str, "META")) gf_bt_force_line(parser);
		else if (!strcmp(str, "IMPORT") || !strcmp(str, "EXPORT")) {
			gf_bt_report(parser, GF_NOT_SUPPORTED, "X3D IMPORT/EXPORT not implemented");
			break;
		}
		else if (!strcmp(str, "InitialObjectDescriptor") || !strcmp(str, "ObjectDescriptor")) {
			parser->load->ctx->root_od = (GF_ObjectDescriptor *) gf_bt_parse_descriptor(parser, str);
		}
		else if (!strcmp(str, "AT") || !strcmp(str, "RAP")) {
			parser->au_is_rap = 0;
			if (!strcmp(str, "RAP")) {
				parser->au_is_rap = 1;
				str = gf_bt_get_next(parser, 0);
				if (strcmp(str, "AT")) {
					gf_bt_report(parser, GF_BAD_PARAM, "AT expected got %s", str);
					parser->last_error = GF_BAD_PARAM;
					break;
				}
			}
			force_new_com = 0;
			str = gf_bt_get_next(parser, 0);
			if (str[0] == 'D') {
				parser->au_time += atoi(&str[1]);
			} else {
				if (sscanf(str, "%u", &parser->au_time) != 1) {
					gf_bt_report(parser, GF_BAD_PARAM, "Number expected got %s", str);
					break;
				}
			}
			if (parser->last_error) break;
			if (parser->od_au && (parser->od_au->timing != parser->au_time)) parser->od_au = NULL;
			if (parser->bifs_au && (parser->bifs_au->timing != parser->au_time)) {
				gf_bt_check_unresolved_nodes(parser);
				parser->bifs_au = NULL;
			}
			parser->stream_id = 0;
			if (!parser->au_time) parser->au_is_rap = 1;
			parser->in_com = 1;
			if (!gf_bt_check_code(parser, '{')) {
				str = gf_bt_get_next(parser, 0);
				if (!strcmp(str, "IN")) {
					gf_bt_parse_int(parser, "IN", (SFInt32*)&parser->stream_id);
					if (parser->last_error) break;
				}
				if (!gf_bt_check_code(parser, '{')) {
					gf_bt_report(parser, GF_BAD_PARAM, "{ expected");
				}
			}
			if (init_com && parser->au_time) break;
		}
		else if (!strcmp(str, "PROTO") || !strcmp(str, "EXTERNPROTO")) {
			gf_bt_parse_proto(parser, str, init_com ? init_com->new_proto_list : NULL);
		}
		else if (!strcmp(str, "NULL")) {
		}
		else if (!strcmp(str, "DEF")) {
			str = gf_bt_get_next(parser, 0);
			strcpy(szDEFName, str);
			has_id = 1;
		}
		else if (!strcmp(str, "ROUTE")) {
			GF_Command *com = NULL;
			if (!parser->top_nodes && parser->bifs_au && !parser->is_wrl) {
				com = gf_sg_command_new(parser->load->scene_graph, GF_SG_ROUTE_INSERT);
				gf_list_add(parser->bifs_au->commands, com);
				gf_list_add(parser->inserted_routes, com);
			}
			r = gf_bt_parse_route(parser, 1, 0, com);
			if (has_id) {
				u32 rID = gf_bt_get_route(parser, szDEFName);
				if (!rID) rID = gf_bt_get_next_route_id(parser);
				if (com) {
					com->RouteID = rID;
					com->def_name = gf_strdup(szDEFName);
					gf_sg_set_max_defined_route_id(parser->load->scene_graph, rID);
				} else if (r) {
					gf_sg_route_set_id(r, rID);
					gf_sg_route_set_name(r, szDEFName);
				}
				has_id = 0;
			}
		}
		else if (!strcmp(str, "UPDATE") || !strcmp(str, "REMOVE")) {
			Bool is_base_stream = parser->stream_id ? 0 : 1;
			if (!parser->stream_id || parser->stream_id==parser->bifs_es->ESID) parser->stream_id = parser->base_od_id;
			if (parser->od_es && (parser->od_es->ESID != parser->stream_id)) {
				GF_StreamContext *prev = parser->od_es;
				parser->od_es = gf_sm_stream_new(parser->load->ctx, (u16) parser->stream_id, GF_STREAM_OD, GF_CODECID_OD_V1);
				if (parser->od_es != prev) {
					parser->bifs_au = NULL;
					parser->od_au = NULL;
				}
			}
			if (!parser->od_es) parser->od_es = gf_sm_stream_new(parser->load->ctx, (u16) parser->stream_id, GF_STREAM_OD, GF_CODECID_OD_V1);
			if (!parser->od_au) parser->od_au = gf_sm_stream_au_new(parser->od_es, parser->au_time, 0, parser->au_is_rap);
			gf_bt_parse_od_command(parser, str);
			if (is_base_stream) parser->stream_id= 0;
		}
		else if (!strcmp(str, "REPLACE") || !strcmp(str, "INSERT") || !strcmp(str, "APPEND") || !strcmp(str, "DELETE")
		         || !strcmp(str, "GLOBALQP") || !strcmp(str, "MULTIPLEREPLACE") || !strcmp(str, "MULTIPLEINDREPLACE") || !strcmp(str, "XDELETE") || !strcmp(str, "DELETEPROTO") || !strcmp(str, "INSERTPROTO")
		         || !strcmp(str, "XREPLACE")
		        ) {
			Bool is_base_stream = parser->stream_id ? 0 : 1;
			if (!parser->stream_id) parser->stream_id = parser->base_bifs_id;
			if (!parser->stream_id || (parser->od_es && (parser->stream_id==parser->od_es->ESID)) ) parser->stream_id = parser->base_bifs_id;
			if (parser->bifs_es->ESID != parser->stream_id) {
				GF_StreamContext *prev = parser->bifs_es;
				parser->bifs_es = gf_sm_stream_new(parser->load->ctx, (u16) parser->stream_id, GF_STREAM_SCENE, GF_CODECID_BIFS);
				if (parser->bifs_es != prev) {
					gf_bt_check_unresolved_nodes(parser);
					parser->bifs_au = NULL;
				}
			}
			if (force_new_com) {
				force_new_com = 0;
				parser->bifs_au = gf_list_last(parser->bifs_es->AUs);
				parser->au_time = (u32) (parser->bifs_au ? parser->bifs_au->timing : 0) + 1;
				parser->bifs_au = NULL;
			}
			if (!parser->bifs_au) parser->bifs_au = gf_sm_stream_au_new(parser->bifs_es, parser->au_time, 0, parser->au_is_rap);
			gf_bt_parse_bifs_command(parser, str, parser->bifs_au->commands);
			if (is_base_stream) parser->stream_id= 0;
		}
		else if (!strcmp(str, "OrderedGroup")
		         || !strcmp(str, "Group")
		         || !strcmp(str, "Layer2D")
		         || !strcmp(str, "Layer3D")
		         || parser->is_wrl
		        )
		{
			node = gf_bt_sf_node(parser, str, vrml_root_node, has_id ? szDEFName : NULL);
			has_id = 0;
			if (!node) break;
			if (parser->top_nodes) {
				gf_list_add(parser->top_nodes, node);
			} else if (!vrml_root_node) {
				if (init_com) init_com->node = node;
				else if (parser->load->flags & GF_SM_LOAD_CONTEXT_READY) {
					GF_Command *com = gf_sg_command_new(parser->load->scene_graph, GF_SG_SCENE_REPLACE);
					assert(!parser->bifs_au);
					assert(parser->bifs_es);
					parser->bifs_au = gf_sm_stream_au_new(parser->bifs_es, 0, 0, 1);
					gf_list_add(parser->bifs_au->commands, com);
					com->node = node;
				}
			} else {
				gf_node_insert_child(vrml_root_node, node, -1);
			}
		}
		else {
			if ( gf_bt_check_code(parser, '}')) parser->in_com = 0;
			else if (strlen(str)) {
				gf_bt_report(parser, GF_BAD_PARAM, "%s: Unknown top-level element", str);
			}
			parser->au_is_rap = 0;
		}
	}
	gf_bt_resolve_routes(parser, 0);
	gf_bt_check_unresolved_nodes(parser);
	while (gf_list_count(parser->scripts)) {
		GF_Node *n = (GF_Node *)gf_list_get(parser->scripts, 0);
		gf_list_rem(parser->scripts, 0);
		gf_sg_script_load(n);
	}
	return parser->last_error;