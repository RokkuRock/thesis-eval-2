static GF_Err dasher_configure_pid(GF_Filter *filter, GF_FilterPid *pid, Bool is_remove)
{
	Bool period_switch = GF_FALSE;
	const GF_PropertyValue *p, *dsi=NULL;
	u32 dc_crc, dc_enh_crc;
	GF_Err e;
	GF_DashStream *ds;
	Bool old_period_switch;
	u32 prev_stream_type;
	Bool new_period_request = GF_FALSE;
	const char *cue_file=NULL;
	s64 old_clamp_dur = 0;
	GF_DasherCtx *ctx = gf_filter_get_udta(filter);
	if (is_remove) {
		ds = gf_filter_pid_get_udta(pid);
		if (ds) {
			if (ds->dyn_bitrate) dasher_update_bitrate(ctx, ds);
			gf_list_del_item(ctx->pids, ds);
			gf_list_del_item(ctx->current_period->streams, ds);
			if (ctx->next_period)
				gf_list_del_item(ctx->next_period->streams, ds);
			dasher_reset_stream(filter, ds, GF_TRUE);
			gf_free(ds);
		}
		return GF_OK;
	}
	ctx->check_connections = GF_TRUE;
	if (!ctx->opid && !ctx->gencues) {
		u32 i, nb_opids = ctx->dual ? 2 : 1;
		for (i=0; i < nb_opids; i++) {
			char *segext=NULL;
			char *force_ext=NULL;
			GF_FilterPid *opid;
			if (i==0) {
				ctx->opid = gf_filter_pid_new(filter);
				gf_filter_pid_set_name(ctx->opid, "MANIFEST");
				opid = ctx->opid;
			} else {
				if (!ctx->alt_dst && ctx->out_path) {
					char szSRC[100];
					GF_FileIO *gfio = NULL;
					char *mpath = ctx->out_path;
					u32 len;
					if (!strncmp(mpath, "gfio://", 7)) {
						gfio = gf_fileio_from_url(mpath);
						if (!gfio) return GF_BAD_PARAM;
						mpath = (char *) gf_file_basename(gf_fileio_resource_url(gfio));
						if (!mpath) return GF_OUT_OF_MEM;
					}
					len = (u32) strlen(mpath);
					char *out_path = gf_malloc(len+10);
					if (!out_path) return GF_OUT_OF_MEM;
					memcpy(out_path, mpath, len);
					out_path[len]=0;
					char *sep = gf_file_ext_start(out_path);
					if (sep) sep[0] = 0;
					if (ctx->do_m3u8) {
						strcat(out_path, ".mpd");
						force_ext = "mpd";
					} else {
						ctx->opid_alt_m3u8 = GF_TRUE;
						ctx->do_m3u8 = GF_TRUE;
						strcat(out_path, ".m3u8");
						force_ext = "m3u8";
					}
					if (gfio) {
						const char *rel = gf_fileio_factory(gfio, out_path);
						gf_free(out_path);
						out_path = gf_strdup(rel);
						if (!out_path) return GF_OUT_OF_MEM;
					}
					ctx->alt_dst = gf_filter_connect_destination(filter, out_path, &e);
					if (e) {
						GF_LOG(GF_LOG_ERROR, GF_LOG_DASH, ("[Dasher] Couldn't create secondary manifest output %s: %s\n", out_path, gf_error_to_string(e) ));
						gf_free(out_path);
						break;
					}
					gf_free(out_path);
					gf_filter_reset_source(ctx->alt_dst);
					snprintf(szSRC, 100, "MuxSrc%cdasher_%p", gf_filter_get_sep(filter, GF_FS_SEP_NAME), ctx->alt_dst);
					gf_filter_set_source(ctx->alt_dst, filter, szSRC);
					ctx->opid_alt = gf_filter_pid_new(filter);
					gf_filter_pid_set_name(ctx->opid_alt, "MANIFEST_ALT");
					snprintf(szSRC, 100, "dasher_%p", ctx->alt_dst);
					gf_filter_pid_set_property(ctx->opid_alt, GF_PROP_PID_MUX_SRC, &PROP_STRING(szSRC) );
					snprintf(szSRC, 100, "dasher_%p", ctx);
					gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_MUX_SRC, &PROP_STRING(szSRC) );
				}
				opid = ctx->opid_alt;
			}
			if (!opid)
				continue;
			gf_filter_pid_copy_properties(opid, pid);
			gf_filter_pid_set_property(opid, GF_PROP_PID_DECODER_CONFIG, NULL);
			gf_filter_pid_set_property(opid, GF_PROP_PID_DECODER_CONFIG_ENHANCEMENT, NULL);
			gf_filter_pid_set_property(opid, GF_PROP_PID_CODECID, NULL);
			gf_filter_pid_set_property(opid, GF_PROP_PID_UNFRAMED, NULL);
			gf_filter_pid_set_property(opid, GF_PROP_PID_STREAM_TYPE, &PROP_UINT(GF_STREAM_FILE) );
			gf_filter_pid_set_property(opid, GF_PROP_PID_ORIG_STREAM_TYPE, &PROP_UINT(GF_STREAM_FILE) );
			gf_filter_pid_set_property(opid, GF_PROP_PID_IS_MANIFEST, &PROP_BOOL(GF_TRUE));
			dasher_check_outpath(ctx);
			p = gf_filter_pid_caps_query(pid, GF_PROP_PID_FILE_EXT);
			if (p) {
				gf_filter_pid_set_property(opid, GF_PROP_PID_FILE_EXT, p );
				segext = p->value.string;
			} else {
				segext = NULL;
				if (ctx->out_path) {
					segext = gf_file_ext_start(ctx->out_path);
				} else if (ctx->mname) {
					segext = gf_file_ext_start(ctx->mname);
				}
				if (!segext) segext = "mpd";
				else segext++;
				if (force_ext)
					segext = force_ext;
				gf_filter_pid_set_property(opid, GF_PROP_PID_FILE_EXT, &PROP_STRING(segext) );
				if (!strcmp(segext, "m3u8")) {
					gf_filter_pid_set_property(opid, GF_PROP_PID_MIME, &PROP_STRING("video/mpegurl"));
				} else if (!strcmp(segext, "ghi")) {
					gf_filter_pid_set_property(opid, GF_PROP_PID_MIME, &PROP_STRING("application/x-gpac-ghi"));
				} else if (!strcmp(segext, "ghix")) {
					gf_filter_pid_set_property(opid, GF_PROP_PID_MIME, &PROP_STRING("application/x-gpac-ghix"));
				} else {
					gf_filter_pid_set_property(opid, GF_PROP_PID_MIME, &PROP_STRING("application/dash+xml"));
				}
			}
			if (!strcmp(segext, "m3u8")) {
				ctx->do_m3u8 = GF_TRUE;
				gf_filter_pid_set_name(opid, "manifest_m3u8" );
			} else if (!strcmp(segext, "ghix") || !strcmp(segext, "ghi")) {
				ctx->do_index = !strcmp(segext, "ghix") ? 2 : 1;
				ctx->sigfrag = GF_FALSE;
				ctx->align = ctx->sap = GF_TRUE;
				ctx->sseg = ctx->sfile = ctx->tpl = GF_FALSE;
				if (ctx->state) {
					gf_free(ctx->state);
					ctx->state = NULL;
					GF_LOG(GF_LOG_WARNING, GF_LOG_DASH, ("[Dasher] Index generation mode, disabling state\n" ));
				}
				if (!ctx->template)
					ctx->template = gf_strdup("$RepresentationID$-$Number$$Init=init$");
				gf_filter_pid_set_name(opid, "dash_index" );
			} else {
				ctx->do_mpd = GF_TRUE;
				gf_filter_pid_set_name(opid, "manifest_mpd" );
			}
		}
		ctx->store_seg_states = GF_FALSE;
		if (((ctx->state || ctx->purge_segments) && !ctx->sseg) || ctx->do_m3u8) ctx->store_seg_states = GF_TRUE;
	}
	ds = gf_filter_pid_get_udta(pid);
	if (!ds) {
		GF_SAFEALLOC(ds, GF_DashStream);
		if (!ds) return GF_OUT_OF_MEM;
		ds->ipid = pid;
		gf_list_add(ctx->pids, ds);
		ds->complementary_streams = gf_list_new();
		period_switch = GF_TRUE;
		gf_filter_pid_set_udta(pid, ds);
		ds->sbound = ctx->sbound;
		ds->startNumber = 1;
		if (ctx->sbound!=DASHER_BOUNDS_OUT)
			ds->packet_queue = gf_list_new();
		if (ctx->is_playing) {
			GF_FilterEvent evt;
			dasher_send_encode_hints(ctx, ds);
			GF_FEVT_INIT(evt, GF_FEVT_PLAY, ds->ipid);
			evt.play.speed = 1.0;
			gf_filter_pid_send_event(ds->ipid, &evt);
		}
		if (ctx->gencues) {
			ds->opid = gf_filter_pid_new(filter);
			gf_filter_pid_copy_properties(ds->opid, pid);
			gf_filter_pid_set_property(ds->opid, GF_PROP_PID_DASH_CUE, &PROP_STRING("inband") );
		}
	}
	gf_filter_pid_set_framing_mode(pid, GF_TRUE);
#define CHECK_PROP(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && (p->value.uint != _mem) && _mem) period_switch = GF_TRUE; \
	if (p) _mem = p->value.uint; \
#define CHECK_PROPL(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && (p->value.longuint != _mem) && _mem) period_switch = GF_TRUE; \
	if (p) _mem = p->value.longuint; \
#define CHECK_PROP_BOOL(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && (p->value.boolean != _mem) && _mem) period_switch = GF_TRUE; \
	if (p) _mem = p->value.uint; \
#define CHECK_PROP_FRAC(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && (p->value.frac.num * _mem.den != p->value.frac.den * _mem.num) && _mem.den && _mem.num) period_switch = GF_TRUE; \
	if (p) _mem = p->value.frac; \
#define CHECK_PROP_FRAC64(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && (p->value.lfrac.num * _mem.den != p->value.lfrac.den * _mem.num) && _mem.den && _mem.num) period_switch = GF_TRUE; \
	if (p) _mem = p->value.lfrac; \
#define CHECK_PROP_STR(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p && p->value.string && _mem && strcmp(_mem, p->value.string)) period_switch = GF_TRUE; \
	if (p) { \
		if (_mem) gf_free(_mem); \
		_mem = gf_strdup(p->value.string); \
	}\
#define CHECK_PROP_PROP(_type, _mem, _e) \
	p = gf_filter_pid_get_property(pid, _type); \
	if (!p && (_e<=0) ) return _e; \
	if (p != _mem) period_switch = GF_TRUE;\
	_mem = p; \
	prev_stream_type = ds->stream_type;
	CHECK_PROP(GF_PROP_PID_STREAM_TYPE, ds->stream_type, GF_NOT_SUPPORTED)
	if (ctx->sigfrag) {
		p = gf_filter_pid_get_property_str(pid, "nofrag");
		if (p && p->value.boolean) {
			p = gf_filter_pid_get_property(pid, GF_PROP_PID_URL);
			GF_LOG(GF_LOG_ERROR, GF_LOG_CONTAINER, ("[IsoMedia] sigfrag requested but file %s is not fragmented\n", p->value.string));
			return GF_BAD_PARAM;
		}
	}
	ds->tile_base = GF_FALSE;
	if (ds->stream_type != GF_STREAM_FILE) {
		u32 prev_bitrate = ds->bitrate;
		if (ds->stream_type==GF_STREAM_ENCRYPTED) {
			CHECK_PROP(GF_PROP_PID_ORIG_STREAM_TYPE, ds->stream_type, GF_EOS)
			ds->is_encrypted = GF_TRUE;
		}
		if (prev_stream_type==ds->stream_type)
			period_switch = GF_FALSE;
		CHECK_PROP(GF_PROP_PID_BITRATE, ds->bitrate, GF_EOS)
		if (!ds->bitrate && prev_bitrate) {
			ds->bitrate = prev_bitrate;
			period_switch = GF_FALSE;
		}
		if (ds->bitrate && period_switch) {
			if ((ds->bitrate <= 120 * prev_bitrate / 100) && (ds->bitrate >= 80 * prev_bitrate / 100)) {
				period_switch = GF_FALSE;
			}
		}
		CHECK_PROP(GF_PROP_PID_CODECID, ds->codec_id, GF_NOT_SUPPORTED)
		CHECK_PROP(GF_PROP_PID_TIMESCALE, ds->timescale, GF_NOT_SUPPORTED)
		if (ds->stream_type==GF_STREAM_VISUAL) {
			CHECK_PROP(GF_PROP_PID_WIDTH, ds->width, GF_EOS)
			CHECK_PROP(GF_PROP_PID_HEIGHT, ds->height, GF_EOS)
			CHECK_PROP_FRAC(GF_PROP_PID_SAR, ds->sar, GF_EOS)
			if (!ds->sar.num) ds->sar.num = ds->sar.den = 1;
			CHECK_PROP_FRAC(GF_PROP_PID_FPS, ds->fps, GF_EOS)
			p = gf_filter_pid_get_property(pid, GF_PROP_PID_TILE_BASE);
			if (p) {
				ds->srd.x = ds->srd.y = 0;
				ds->srd.z = ds->width;
				ds->srd.w = ds->height;
				ds->tile_base = GF_TRUE;
			} else {
				p = gf_filter_pid_get_property(pid, GF_PROP_PID_CROP_POS);
				if (p && ((p->value.vec2i.x != ds->srd.x) || (p->value.vec2i.y != ds->srd.y) ) ) period_switch = GF_TRUE;
				if (p) {
					ds->srd.x = p->value.vec2i.x;
					ds->srd.y = p->value.vec2i.y;
					ds->srd.z = ds->width;
					ds->srd.w = ds->height;
				} else {
					p = gf_filter_pid_get_property(pid, GF_PROP_PID_SRD);
					if (p && (
						(p->value.vec4i.x != ds->srd.x)
						|| (p->value.vec4i.y != ds->srd.y)
						|| (p->value.vec4i.z != ds->srd.z)
						|| (p->value.vec4i.w != ds->srd.w)
					) )
						period_switch = GF_TRUE;
					if (p) {
						ds->srd.x = p->value.vec4i.x;
						ds->srd.y = p->value.vec4i.y;
						ds->srd.z = p->value.vec4i.z;
						ds->srd.w = p->value.vec4i.w;
					}
				}
			}
		} else if (ds->stream_type==GF_STREAM_AUDIO) {
			CHECK_PROP(GF_PROP_PID_SAMPLE_RATE, ds->sr, GF_EOS)
			CHECK_PROP(GF_PROP_PID_NUM_CHANNELS, ds->nb_ch, GF_EOS)
			CHECK_PROPL(GF_PROP_PID_CHANNEL_LAYOUT, ds->ch_layout, GF_EOS)
		}
		old_period_switch = period_switch;
		CHECK_PROP(GF_PROP_PID_NB_FRAMES, ds->nb_samples_in_source, GF_EOS)
		CHECK_PROP_FRAC64(GF_PROP_PID_DURATION, ds->duration, GF_EOS)
		CHECK_PROP_STR(GF_PROP_PID_URL, ds->src_url, GF_EOS)
		period_switch = old_period_switch;
		if (ds->duration.num<0) ds->duration.num = 0;
		CHECK_PROP(GF_PROP_PID_ID, ds->id, GF_EOS)
		CHECK_PROP(GF_PROP_PID_DEPENDENCY_ID, ds->dep_id, GF_EOS)
		p = gf_filter_pid_get_property(pid, GF_PROP_PID_HAS_SYNC);
		u32 sync_type = DASHER_SYNC_UNKNOWN;
		if (p) sync_type = p->value.boolean ? DASHER_SYNC_PRESENT : DASHER_SYNC_NONE;
		if (sync_type != ds->sync_points_type) period_switch = GF_TRUE;
		ds->sync_points_type = sync_type;
		if (ds->inband_cues)
			period_switch = old_period_switch;
		if (ctx->scope_deps) {
			const char *src_args = gf_filter_pid_orig_src_args(pid, GF_TRUE);
			if (src_args) {
				ds->src_id = gf_crc_32(src_args, (u32) strlen(src_args));
			}
		}
		if (ctx->pswitch==DASHER_PSWITCH_STSD) {
			p = gf_filter_pid_get_property(pid, GF_PROP_PID_ISOM_STSD_ALL_TEMPLATES);
			if (p) {
				u32 all_stsd_crc = gf_crc_32(p->value.data.ptr, p->value.data.size);
				if (all_stsd_crc==ds->all_stsd_crc) {
					ds->dsi_crc = 0;
					ds->dsi_enh_crc = 0;
				} else {
					ds->all_stsd_crc = all_stsd_crc;
				}
			} else {
				ds->all_stsd_crc = 0;
			}
		}
		dc_crc = 0;
		dsi = p = gf_filter_pid_get_property(pid, GF_PROP_PID_DECODER_CONFIG);
		if (p && (p->type==GF_PROP_DATA))
			dc_crc = gf_crc_32(p->value.data.ptr, p->value.data.size);
		dc_enh_crc = 0;
		p = gf_filter_pid_get_property(pid, GF_PROP_PID_DECODER_CONFIG_ENHANCEMENT);
		if (p && (p->type==GF_PROP_DATA)) dc_enh_crc = gf_crc_32(p->value.data.ptr, p->value.data.size);
		if (((dc_crc != ds->dsi_crc) && ds->dsi_crc)
			|| ((dc_enh_crc != ds->dsi_enh_crc) && ds->dsi_enh_crc)
		) {
			switch (ds->codec_id) {
			case GF_CODECID_AVC:
			case GF_CODECID_SVC:
			case GF_CODECID_MVC:
			case GF_CODECID_HEVC:
			case GF_CODECID_LHVC:
				if (!ctx->bs_switch)
					period_switch = GF_TRUE;
				break;
			default:
				period_switch = GF_TRUE;
				break;
			}
		}
		ds->dcd_not_ready = 0;
		if (!dc_crc && !dc_enh_crc) {
			switch (ds->codec_id) {
			case GF_CODECID_AVC:
			case GF_CODECID_SVC:
			case GF_CODECID_MVC:
			case GF_CODECID_HEVC:
			case GF_CODECID_LHVC:
			case GF_CODECID_AAC_MPEG4:
			case GF_CODECID_AAC_MPEG2_MP:
			case GF_CODECID_AAC_MPEG2_LCP:
			case GF_CODECID_AAC_MPEG2_SSRP:
			case GF_CODECID_USAC:
			case GF_CODECID_AC3:
			case GF_CODECID_EAC3:
			case GF_CODECID_AV1:
			case GF_CODECID_VP8:
			case GF_CODECID_VP9:
				ds->dcd_not_ready = gf_sys_clock();
				break;
			default:
				break;
			}
		}
		ds->dsi_crc = dc_crc;
		CHECK_PROP_STR(GF_PROP_PID_TEMPLATE, ds->template, GF_EOS)
		CHECK_PROP_STR(GF_PROP_PID_LANGUAGE, ds->lang, GF_EOS)
		CHECK_PROP_BOOL(GF_PROP_PID_INTERLACED, ds->interlaced, GF_EOS)
		CHECK_PROP_PROP(GF_PROP_PID_AS_COND_DESC, ds->p_as_desc, GF_EOS)
		CHECK_PROP_PROP(GF_PROP_PID_AS_ANY_DESC, ds->p_as_any_desc, GF_EOS)
		CHECK_PROP_PROP(GF_PROP_PID_REP_DESC, ds->p_rep_desc, GF_EOS)
		CHECK_PROP_PROP(GF_PROP_PID_BASE_URL, ds->p_base_url, GF_EOS)
		CHECK_PROP_PROP(GF_PROP_PID_ROLE, ds->p_role, GF_EOS)
		CHECK_PROP_STR(GF_PROP_PID_HLS_PLAYLIST, ds->hls_vp_name, GF_EOS)
		CHECK_PROP_BOOL(GF_PROP_PID_SINGLE_SCALE, ds->sscale, GF_EOS)
		if (ctx->sigfrag && ctx->tpl && !ctx->template && !ds->template) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_DASH, ("[Dasher] Warning, manifest generation only mode requested for live-based profile but no template provided, switching to main profile.\n"));
			ctx->profile = GF_DASH_PROFILE_MAIN;
			ctx->tpl = GF_FALSE;
			dasher_setup_profile(ctx);
			ctx->sfile = GF_TRUE;
		}
		if (ds->rate_first_dts_plus_one)
			dasher_update_bitrate(ctx, ds);
		if (!ds->bitrate) {
			char *tpl = ds->template ? ds->template : ctx->template;
			if (tpl && strstr(tpl, "$Bandwidth$")) {
				GF_LOG(GF_LOG_ERROR, GF_LOG_DASH, ("[Dasher] No bitrate property assigned to PID %s but template uses $Bandwidth$, cannot initialize !\n\tTry specifying bitrate property after your source, e.g. -i source.raw:#Bitrate=VAL\n", gf_filter_pid_get_name(ds->ipid)));
				ctx->in_error = GF_TRUE;
				return GF_BAD_PARAM;
			} else {
				GF_LOG(GF_LOG_INFO, GF_LOG_DASH, ("[Dasher] No bitrate property assigned to PID %s, computing from bitstream\n", gf_filter_pid_get_name(ds->ipid)));
				ds->dyn_bitrate = GF_TRUE;
				ds->rate_first_dts_plus_one = 0;
				ds->rate_media_size = 0;
			}
		} else {
			ds->dyn_bitrate = GF_FALSE;
		}
		if (!ds->src_url)
			ds->src_url = gf_strdup("file");
		CHECK_PROP(GF_PROP_PID_START_NUMBER, ds->startNumber, GF_EOS)
		ds->no_seg_dur = ctx->no_seg_dur;
		dasher_get_dash_dur(ctx, ds);
		ds->splitable = GF_FALSE;
		ds->is_av = GF_FALSE;
		switch (ds->stream_type) {
		case GF_STREAM_TEXT:
		case GF_STREAM_METADATA:
		case GF_STREAM_OD:
		case GF_STREAM_SCENE:
			ds->splitable = ctx->split;
			break;
		case GF_STREAM_VISUAL:
		case GF_STREAM_AUDIO:
			ds->is_av = GF_TRUE;
			break;
		}
		old_clamp_dur = ds->clamped_dur.num;
		ds->clamped_dur.num = 0;
		ds->clamped_dur.den = 1;
		p = gf_filter_pid_get_property(pid, GF_PROP_PID_CLAMP_DUR);
		if (p && p->value.lfrac.den) ds->clamped_dur = p->value.lfrac;
#if !defined(GPAC_DISABLE_AV_PARSERS)
		if (dsi) {
			if (ds->codec_id == GF_CODECID_LHVC || ds->codec_id == GF_CODECID_HEVC_TILES || ds->codec_id == GF_CODECID_HEVC) {
				GF_HEVCConfig* hevccfg = gf_odf_hevc_cfg_read(dsi->value.data.ptr, dsi->value.data.size, GF_FALSE);
				if (hevccfg) {
					Bool is_interlaced;
					HEVCState hevc;
					HEVC_SPS* sps;
					memset(&hevc, 0, sizeof(HEVCState));
					gf_hevc_parse_ps(hevccfg, &hevc, GF_HEVC_NALU_VID_PARAM);
					gf_hevc_parse_ps(hevccfg, &hevc, GF_HEVC_NALU_SEQ_PARAM);
					sps = &hevc.sps[hevc.sps_active_idx];
					if (sps && sps->colour_description_present_flag) {
						DasherHDRType old_hdr_type = ds->hdr_type;
						if (sps->colour_primaries == 9 && sps->matrix_coeffs == 9) {
							if (sps->transfer_characteristic == 14) ds->hdr_type = DASHER_HDR_HLG;  
							if (sps->transfer_characteristic == 16) ds->hdr_type = DASHER_HDR_PQ10;
						}
						if (old_hdr_type != ds->hdr_type) period_switch = GF_TRUE;
					}
					is_interlaced = hevccfg->interlaced_source_flag ? GF_TRUE : GF_FALSE;
					if (ds->interlaced != is_interlaced) period_switch = GF_TRUE;
					ds->interlaced = is_interlaced;
					gf_odf_hevc_cfg_del(hevccfg);
				}
			}
			else if (ds->codec_id == GF_CODECID_AVC || ds->codec_id == GF_CODECID_SVC || ds->codec_id == GF_CODECID_MVC) {
				AVCState avc;
				GF_AVCConfig* avccfg = gf_odf_avc_cfg_read(dsi->value.data.ptr, dsi->value.data.size);
				GF_NALUFFParam *sl = (GF_NALUFFParam *)gf_list_get(avccfg->sequenceParameterSets, 0);
				if (sl) {
					s32 idx;
					memset(&avc, 0, sizeof(AVCState));
					idx = gf_avc_read_sps(sl->data, sl->size, &avc, 0, NULL);
					if (idx>=0) {
						Bool is_interlaced = avc.sps[idx].frame_mbs_only_flag ? GF_FALSE : GF_TRUE;
						if (ds->interlaced != is_interlaced) period_switch = GF_TRUE;
						ds->interlaced = is_interlaced;
					}
				}
				gf_odf_avc_cfg_del(avccfg);
			}
		}
#endif  
		if (ds->stream_type==GF_STREAM_AUDIO) {
			u32 _sr=0, _nb_ch=0;
#ifndef GPAC_DISABLE_AV_PARSERS
			switch (ds->codec_id) {
			case GF_CODECID_AAC_MPEG4:
			case GF_CODECID_AAC_MPEG2_MP:
			case GF_CODECID_AAC_MPEG2_LCP:
			case GF_CODECID_AAC_MPEG2_SSRP:
			case GF_CODECID_USAC:
				if ((ctx->profile == GF_DASH_PROFILE_AVC264_LIVE)
					|| (ctx->profile == GF_DASH_PROFILE_AVC264_ONDEMAND)
					|| (ctx->profile == GF_DASH_PROFILE_DASHIF_LL)
				) {
					GF_Err res = dasher_get_audio_info_with_m4a_sbr_ps(ds, dsi, &_sr, &_nb_ch);
					if (res) {
						GF_LOG(GF_LOG_ERROR, GF_LOG_DASH, ("[Dasher] Could not get AAC info, %s\n", gf_error_to_string(res)));
					}
				} else if (dsi) {
					dasher_get_audio_info_with_m4a_sbr_ps(ds, dsi, NULL, &_nb_ch);
				}
				break;
			case GF_CODECID_AC3:
			case GF_CODECID_EAC3:
				if (dsi) {
					GF_AC3Config ac3;
					gf_odf_ac3_config_parse(dsi->value.data.ptr, dsi->value.data.size, (ds->codec_id==GF_CODECID_EAC3) ? GF_TRUE : GF_FALSE, &ac3);
					ds->nb_lfe = ac3.streams[0].lfon ? 1 : 0;
					ds->nb_surround = gf_ac3_get_surround_channels(ac3.streams[0].acmod);
					ds->atmos_complexity_type = ac3.is_ec3 ? ac3.complexity_index_type : 0;
					_nb_ch = gf_ac3_get_total_channels(ac3.streams[0].acmod);
					if (ac3.streams[0].nb_dep_sub) {
						_nb_ch += gf_eac3_get_chan_loc_count(ac3.streams[0].chan_loc);
					}
                    if (ds->nb_lfe) _nb_ch++;
				}
				break;
			}
#endif
			if (_sr > ds->sr) ds->sr = _sr;
			if (_nb_ch > ds->nb_ch) ds->nb_ch = _nb_ch;
		}
		ds->pts_minus_cts = 0;
		p = gf_filter_pid_get_property(ds->ipid, GF_PROP_PID_DELAY);
		if (p && p->value.longsint) {
			ds->pts_minus_cts = p->value.longsint;
		}
		if (period_switch) {
			cue_file = ctx->cues;
			if (!cue_file || strcmp(cue_file, "none") ) {
				p = gf_filter_pid_get_property(pid, GF_PROP_PID_DASH_CUE);
				if (p) cue_file = p->value.string;
			}
			if (ds->cues) gf_free(ds->cues);
			ds->cues = NULL;
			ds->nb_cues = 0;
			ds->inband_cues = GF_FALSE;
			if (cue_file) {
				if (!strcmp(cue_file, "inband")) {
					ds->inband_cues = GF_TRUE;
					if (!ctx->sigfrag) {
						p = gf_filter_pid_get_property(pid, GF_PROP_PID_DASH_FWD);
						if (p && p->value.uint)
							ctx->forward_mode = p->value.uint;
					}
				} else if (!strcmp(cue_file, "idx_all")) {
					ds->inband_cues = GF_TRUE;
					ctx->from_index = IDXMODE_ALL;
				} else if (!strcmp(cue_file, "idx_man")) {
					ds->inband_cues = GF_TRUE;
					ctx->from_index = IDXMODE_MANIFEST;
				} else if (!strcmp(cue_file, "idx_init")) {
					ds->inband_cues = GF_TRUE;
					ctx->from_index = IDXMODE_INIT;
				} else if (!strcmp(cue_file, "idx_child")) {
					ds->inband_cues = GF_TRUE;
					ctx->from_index = IDXMODE_CHILD;
				} else if (!strcmp(cue_file, "idx_seg")) {
					ds->inband_cues = GF_TRUE;
					ctx->from_index = IDXMODE_SEG;
				} else if (strcmp(cue_file, "none")) {
					e = gf_mpd_load_cues(cue_file, ds->id, &ds->cues_timescale, &ds->cues_use_edits, &ds->cues_ts_offset, &ds->cues, &ds->nb_cues);
					if (e) return e;
					if (!ds->cues_timescale)
						ds->cues_timescale = ds->timescale;
				}
				if (ctx->from_index==IDXMODE_CHILD) {
					p = gf_filter_pid_get_property_str(ds->ipid, "idx_out");
					if (p) {
						if (ds->hls_vp_name) gf_free(ds->hls_vp_name);
						ds->hls_vp_name = gf_strdup(p->value.string);
					}
				}
			}
		}
	} else {
		p = gf_filter_pid_get_property(pid, GF_PROP_PID_URL);
		if (!p) p = gf_filter_pid_get_property(pid, GF_PROP_PID_FILEPATH);
		if (p) return GF_NOT_SUPPORTED;
		CHECK_PROP_STR(GF_PROP_PID_XLINK, ds->xlink, GF_EOS)
	}
	if (ctx->do_index || ctx->from_index) {
		if (!ds->template && ctx->def_template) {
			p = gf_filter_pid_get_property_str(ds->ipid, "idx_template");
			if (p) {
				ds->template = gf_strdup(p->value.string);
				GF_LOG(GF_LOG_INFO, GF_LOG_DASH, ("[Dasher] Using template from index pass %s\n", ds->template));
			}
		}
		char *template = ds->template;
		if (!ds->template) {
			if ((ctx->def_template==1) && ctx->do_index) {
				gf_free(ctx->template);
				ctx->template = gf_strdup("$RepresentationID$-$Number$$Init=init$");
				ctx->def_template = 2;
				GF_LOG(GF_LOG_INFO, GF_LOG_DASH, ("[Dasher] No template assigned in index mode, using %s\n", ctx->template));
			}
			template = ctx->template;
		}
		if (dasher_template_use_source_url(template)) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_DASH, ("[Dasher] Cannot use file-based templates with index mode\n"));
			return GF_BAD_PARAM;
		}
	}
	if (!ds->rep && (gf_list_find(ctx->current_period->streams, ds)>=0))
		period_switch = GF_FALSE;
	old_period_switch = period_switch;
	period_switch = GF_FALSE;
	CHECK_PROP_STR(GF_PROP_PID_PERIOD_ID, ds->period_id, GF_EOS)
	CHECK_PROP_PROP(GF_PROP_PID_PERIOD_DESC, ds->p_period_desc, GF_EOS)
	if (!period_switch && (ctx->pswitch==DASHER_PSWITCH_FORCE))
		period_switch = GF_TRUE;
	if (gf_filter_pid_get_property_str(pid, "period_switch"))
		period_switch = GF_TRUE;
	p = gf_filter_pid_get_property(pid, GF_PROP_PID_PERIOD_START);
	if (p) {
		if (ds->period_start.num * p->value.lfrac.den != p->value.lfrac.num * ds->period_start.den) period_switch = GF_TRUE;
		ds->period_start = p->value.lfrac;
	} else {
		if (ds->period_start.num) period_switch = GF_TRUE;
		ds->period_start.num = 0;
		ds->period_start.den = 1000;
	}
	assert(ds->period_start.den);
	if (period_switch) {
		new_period_request = GF_TRUE;
	} else {
		period_switch = old_period_switch;
	}
	if (ds->period_continuity_id) gf_free(ds->period_continuity_id);
	ds->period_continuity_id = NULL;
	p = gf_filter_pid_get_property_str(ds->ipid, "period_resume");
	if (!ctx->mpd || (gf_list_find(ctx->mpd->periods, ds->last_period)<0))
		ds->last_period = NULL;
	if (p && p->value.string && ds->last_period) {
		if (!ds->last_period->ID) {
			if (p->value.string[0]) {
				ds->last_period->ID = p->value.string;
			} else {
				char szPName[50];
				sprintf(szPName, "P%d", 1 + gf_list_find(ctx->mpd->periods, ds->last_period));
				ds->last_period->ID = gf_strdup(szPName);
			}
		}
		if (ds->set && (ds->set->id<0)) {
			if (!ds->as_id && ds->period && ds->period->period)
				ds->as_id = gf_list_find(ds->period->period->adaptation_sets, ds->set) + 1;
			ds->set->id = ds->as_id;
		}
		ds->period_continuity_id = gf_strdup(ds->last_period->ID);
	}
	ds->last_period = NULL;
	ds->period_dur.num = 0;
	ds->period_dur.den = 1;
	p = gf_filter_pid_get_property(pid, GF_PROP_PID_PERIOD_DUR);
	if (p) ds->period_dur = p->value.lfrac;
	p = gf_filter_pid_get_property_str(pid, "max_seg_dur");
	ctx->index_max_seg_dur = p ? p->value.uint : 0;
	p = gf_filter_pid_get_property_str(pid, "mpd_duration");
	ctx->index_media_duration = p ? p->value.longuint : 0;
	if (ds->stream_type==GF_STREAM_FILE) {
		if (!ds->xlink && !ds->period_start.num && !ds->period_dur.num) {
			ds->done = 1;
			GF_LOG(GF_LOG_WARNING, GF_LOG_DASH, ("[Dasher] null PID specified without any XLINK/start/duration, ignoring\n"));
		} else if (ds->xlink) {
			ctx->use_xlink = GF_TRUE;
		}
	} else {
		if (ds->xlink) gf_free(ds->xlink);
		ds->xlink = NULL;
		CHECK_PROP_STR(GF_PROP_PID_XLINK, ds->xlink, GF_EOS)
		if (ds->xlink)
			ctx->use_xlink = GF_TRUE;
	}
	if (new_period_request && ds->done && old_clamp_dur) {
		gf_list_del_item(ctx->next_period->streams, ds);
		gf_filter_pid_set_discard(ds->ipid, GF_FALSE);
		if (ds->opid && !ctx->gencues) {
			gf_filter_pid_discard_block(ds->opid);
			gf_filter_pid_remove(ds->opid);
			ds->opid = NULL;
		}
		if (ctx->is_eos) {
			ctx->is_eos = GF_FALSE;
			gf_filter_pid_discard_block(ctx->opid);
			if (ctx->opid_alt)
			gf_filter_pid_discard_block(ctx->opid_alt);
		}
		ds->rep_init = GF_FALSE;
		ds->presentation_time_offset = 0;
		ds->rep = NULL;
		ds->set = NULL;
		ds->period = NULL;
		ds->done = 0;
	}
	if (gf_list_find(ctx->next_period->streams, ds)>=0)
		period_switch = GF_FALSE;
	if (!ds->period_id)
		ds->period_id = gf_strdup(DEFAULT_PERIOD_ID);
	e = dasher_hls_setup_crypto(ctx, ds);
	if (e) return e;
	if (!period_switch) {
		if (ds->opid) {
			gf_filter_pid_copy_properties(ds->opid, pid);
			if (ctx->is_route && ctx->do_m3u8)
				gf_filter_pid_set_property(ds->opid, GF_PROP_PCK_HLS_REF, &PROP_LONGUINT( ds->hls_ref_id ) );
			if (ctx->llhls)
				gf_filter_pid_set_property(ds->opid, GF_PROP_PID_LLHLS, &PROP_UINT(ctx->llhls) );
			if (ctx->gencues)
				gf_filter_pid_set_property(ds->opid, GF_PROP_PID_DASH_CUE, &PROP_STRING("inband") );
		}
		if (ds->rep)
			dasher_update_rep(ctx, ds);
		return GF_OK;
	}