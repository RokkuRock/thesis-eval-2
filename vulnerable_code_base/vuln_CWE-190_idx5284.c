static GF_Err mp4_mux_initialize_movie(GF_MP4MuxCtx *ctx)
{
#ifndef GPAC_DISABLE_ISOM_FRAGMENTS
	GF_Err e;
	u32 i, count = gf_list_count(ctx->tracks);
	TrackWriter *ref_tkw = NULL;
	u64 min_dts = 0;
	u32 min_dts_scale=0;
	u32 def_fake_dur=0;
	u32 def_fake_scale=0;
#ifdef GF_ENABLE_CTRN
	u32 traf_inherit_base_id=0;
#endif
	u32 nb_segments=0;
	GF_Fraction64 max_dur;
	ctx->single_file = GF_TRUE;
	ctx->current_offset = ctx->current_size = 0;
	max_dur.den = 1;
	max_dur.num = 0;
	if (ctx->sseg && ctx->noinit)
		ctx->single_file = GF_FALSE;
	if (ctx->dur.num && ctx->dur.den) {
		max_dur.num = ctx->dur.num;
		max_dur.den = ctx->dur.den;
	}
	for (i=0; i<count; i++) {
		const GF_PropertyValue *p;
		TrackWriter *tkw = gf_list_get(ctx->tracks, i);
		GF_FilterPacket *pck;
		if (tkw->fake_track) continue;
		pck = gf_filter_pid_get_packet(tkw->ipid);
		if (!pck) {
			if (gf_filter_pid_is_eos(tkw->ipid)) {
				if (tkw->dgl_copy) {
					gf_filter_pck_discard(tkw->dgl_copy);
					tkw->dgl_copy = NULL;
				}
				continue;
			}
			return GF_OK;
		}
		if (!ctx->dash_mode && !ctx->cur_file_idx_plus_one) {
			p = gf_filter_pck_get_property(pck, GF_PROP_PCK_FILENUM);
			if (p) {
				ctx->cur_file_idx_plus_one = p->value.uint + 1;
				if (!ctx->cur_file_suffix) {
					p = gf_filter_pck_get_property(pck, GF_PROP_PCK_FILESUF);
					if (p && p->value.string) ctx->cur_file_suffix = gf_strdup(p->value.string);
				}
				ctx->notify_filename = GF_TRUE;
			}
		}
		if (tkw->cenc_state==CENC_NEED_SETUP) {
			mp4_mux_cenc_update(ctx, tkw, pck, CENC_CONFIG, 0, 0);
		}
		p = gf_filter_pck_get_property(pck, GF_PROP_PCK_FILENAME);
		if (p && strlen(p->value.string)) ctx->single_file = GF_FALSE;
		def_fake_dur = gf_filter_pck_get_duration(pck);
		def_fake_scale = tkw->src_timescale;
		p = gf_filter_pid_get_property(tkw->ipid, GF_PROP_PID_DURATION);
		if (p && p->value.lfrac.den) {
			tkw->pid_dur = p->value.lfrac;
			if (tkw->pid_dur.num<0) tkw->pid_dur.num = -tkw->pid_dur.num;
			if (max_dur.num * (s64) tkw->pid_dur.den < (s64) max_dur.den * tkw->pid_dur.num) {
				max_dur.num = tkw->pid_dur.num;
				max_dur.den = tkw->pid_dur.den;
			}
		}
#ifdef GF_ENABLE_CTRN
		if (tkw->codecid==GF_CODECID_HEVC)
			traf_inherit_base_id = tkw->track_id;
#endif
	}
	for (i=0; i<count; i++) {
		u32 def_pck_dur;
		u32 def_samp_size=0;
		u8 def_is_rap;
#ifdef GF_ENABLE_CTRN
		u32 inherit_traf_from_track = 0;
#endif
		u64 dts;
		const GF_PropertyValue *p;
		TrackWriter *tkw = gf_list_get(ctx->tracks, i);
		if (tkw->fake_track) {
			if (def_fake_scale) {
				def_pck_dur = def_fake_dur;
				def_pck_dur *= tkw->src_timescale;
				def_pck_dur /= def_fake_scale;
			} else {
				def_pck_dur = 0;
			}
		} else {
			GF_FilterPacket *pck = gf_filter_pid_get_packet(tkw->ipid);
			if (pck) {
				u32 tscale;
				def_pck_dur = gf_filter_pck_get_duration(pck);
				dts = gf_filter_pck_get_dts(pck);
				if (dts == GF_FILTER_NO_TS)
					dts = gf_filter_pck_get_cts(pck);
				tscale = gf_filter_pck_get_timescale(pck);
				if (!min_dts || gf_timestamp_greater(min_dts, min_dts_scale, dts, tscale)) {
					min_dts = dts;
					min_dts_scale = tscale;
				}
				if (tkw->raw_audio_bytes_per_sample) {
					u32 pck_size;
					gf_filter_pck_get_data(pck, &pck_size);
					pck_size /= tkw->raw_audio_bytes_per_sample;
					if (pck_size)
						def_pck_dur /= pck_size;
				}
			} else {
				p = gf_filter_pid_get_property(tkw->ipid, GF_PROP_PID_CONSTANT_DURATION);
				def_pck_dur = p ? p->value.uint : 0;
			}
			if (tkw->raw_audio_bytes_per_sample)
				def_samp_size = tkw->raw_audio_bytes_per_sample;
		}
		if (tkw->src_timescale != tkw->tk_timescale) {
			def_pck_dur *= tkw->tk_timescale;
			def_pck_dur /= tkw->src_timescale;
		}
		switch (tkw->stream_type) {
		case GF_STREAM_AUDIO:
		case GF_STREAM_TEXT:
			def_is_rap = GF_ISOM_FRAG_DEF_IS_SYNC;
			p = gf_filter_pid_get_property(tkw->ipid, GF_PROP_PID_HAS_SYNC);
			if (p && p->value.boolean)
				def_is_rap = 0;
			break;
		case GF_STREAM_VISUAL:
			switch (tkw->codecid) {
			case GF_CODECID_PNG:
			case GF_CODECID_JPEG:
			case GF_CODECID_J2K:
				break;
			case GF_CODECID_HEVC_TILES:
#ifdef GF_ENABLE_CTRN
				if (ctx->ctrn && ctx->ctrni)
					inherit_traf_from_track = traf_inherit_base_id;
#endif
				break;
			default:
				if (!ref_tkw) ref_tkw = tkw;
				break;
			}
			def_is_rap = 0;
			break;
		default:
			def_is_rap = 0;
			break;
		}
		if (ctx->cmaf && !def_is_rap) {
			def_is_rap |= GF_ISOM_FRAG_USE_SYNC_TABLE;
		}
		mp4_mux_set_hevc_groups(ctx, tkw);
		e = gf_isom_setup_track_fragment(ctx->file, tkw->track_id, tkw->stsd_idx, def_pck_dur, def_samp_size, def_is_rap, 0, 0, ctx->nofragdef ? GF_TRUE : GF_FALSE);
		if (e) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_CONTAINER, ("[MP4Mux] Unable to setup fragmentation for track ID %d: %s\n", tkw->track_id, gf_error_to_string(e) ));
			return e;
		}
#ifndef GPAC_DISABLE_ISOM_FRAGMENTS
		if (ctx->refrag) {
			p = gf_filter_pid_get_property(tkw->ipid, GF_PROP_PID_ISOM_TREX_TEMPLATE);
			if (p) {
				gf_isom_setup_track_fragment_template(ctx->file, tkw->track_id, p->value.data.ptr, p->value.data.size, ctx->nofragdef);
			} else if (!ctx->nofragdef) {
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[MP4Mux] Refragmentation with default track fragment flags signaling but no TREX found in source track %d, using defaults computed from PID, result might be broken\n", tkw->track_id));
			}
		}
#endif
		if (ctx->tfdt.den && ctx->tfdt.num) {
			tkw->tfdt_offset = gf_timestamp_rescale(ctx->tfdt.num, ctx->tfdt.den, tkw->tk_timescale);
		}
		if (tkw->fake_track) {
			gf_list_del_item(ctx->tracks, tkw);
			if (ref_tkw==tkw) ref_tkw=NULL;
			mp4_mux_track_writer_del(tkw);
			i--;
			count--;
			continue;
		}
#ifdef GF_ENABLE_CTRN
		if (inherit_traf_from_track)
			gf_isom_enable_traf_inherit(ctx->file, tkw->track_id, inherit_traf_from_track);
#endif
		if (!tkw->box_patched) {
			p = gf_filter_pid_get_property_str(tkw->ipid, "boxpatch");
			if (p && p->value.string) {
				e = gf_isom_apply_box_patch(ctx->file, tkw->track_id, p->value.string, GF_FALSE);
				if (e) {
					GF_LOG(GF_LOG_ERROR, GF_LOG_CONTAINER, ("[MP4Mux] Unable to apply box patch %s to track %d: %s\n",
						p->value.string, tkw->track_id, gf_error_to_string(e) ));
				}
			}
			tkw->box_patched = GF_TRUE;
		}
		p = gf_filter_pid_get_property(tkw->ipid, GF_PROP_PID_DASH_SEGMENTS);
		if (p && (p->value.uint>nb_segments))
			nb_segments = p->value.uint;
		if (!ctx->dash_mode)
			gf_isom_purge_track_reference(ctx->file, tkw->track_num);
	}
	if (max_dur.num && max_dur.den) {
		u64 mdur = max_dur.num;
		if (ctx->moovts != max_dur.den) {
			mdur *= (u32) ctx->moovts;
			mdur /= max_dur.den;
		}
		gf_isom_set_movie_duration(ctx->file, mdur, GF_FALSE);
	}
	else if (ctx->cmaf) {
		gf_isom_set_movie_duration(ctx->file, 0, GF_TRUE);
	}
	if (ref_tkw) {
		gf_list_del_item(ctx->tracks, ref_tkw);
		gf_list_insert(ctx->tracks, ref_tkw, 0);
	}
	ctx->ref_tkw = gf_list_get(ctx->tracks, 0);
	if (!ctx->abs_offset) {
		u32 mval = ctx->dash_mode ? '6' : '5';
		u32 mbrand, mcount, found=0;
		u8 szB[GF_4CC_MSIZE];
		gf_isom_set_fragment_option(ctx->file, 0, GF_ISOM_TFHD_FORCE_MOOF_BASE_OFFSET, 1);
		gf_isom_get_brand_info(ctx->file, &mbrand, NULL, &mcount);
		strcpy(szB, gf_4cc_to_str(mbrand));
		if (!strncmp(szB, "iso", 3) && (szB[3] >= mval) && (szB[3] <= 'F') ) found = 1;
		i=0;
		while (!found && (i<mcount)) {
			i++;
			gf_isom_get_alternate_brand(ctx->file, i, &mbrand);
			strcpy(szB, gf_4cc_to_str(mbrand));
			if (!strncmp(szB, "iso", 3) && (szB[3] >= mval) && (szB[3] <= 'F') ) found = 1;
		}
		if (!found) {
			gf_isom_set_brand_info(ctx->file, ctx->dash_mode ? GF_ISOM_BRAND_ISO6 : GF_ISOM_BRAND_ISO5, 1);
		}
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_ISOM, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_ISO1, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_ISO2, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_ISO3, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_ISO4, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_AVC1, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_MP41, GF_FALSE);
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_MP42, GF_FALSE);
	}
	if (ctx->dash_mode) {
		if (ctx->dash_mode==MP4MX_DASH_VOD) {
			gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_DSMS, GF_TRUE);
		} else {
			gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_DASH, GF_TRUE);
		}
		gf_isom_modify_alternate_brand(ctx->file, GF_ISOM_BRAND_MSIX, ((ctx->dash_mode==MP4MX_DASH_VOD) && (ctx->subs_sidx>=0)) ? GF_TRUE : GF_FALSE);
	}
	if (ctx->boxpatch && !ctx->box_patched) {
		e = gf_isom_apply_box_patch(ctx->file, 0, ctx->boxpatch, GF_FALSE);
		if (e) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_CONTAINER, ("[MP4Mux] Unable to apply box patch %s: %s\n", ctx->boxpatch, gf_error_to_string(e) ));
		}
		ctx->box_patched = GF_TRUE;
	}
	e = gf_isom_finalize_for_fragment(ctx->file, ctx->dash_mode ? 1 : 0, ctx->mvex);
	if (e) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CONTAINER, ("[MP4Mux] Unable to finalize moov for fragmentation: %s\n", gf_error_to_string(e) ));
		return e;
	}
	ctx->init_movie_done = GF_TRUE;
	if (min_dts_scale) {
		u64 rs_dts = gf_timestamp_rescale(min_dts, min_dts_scale, ctx->cdur.den);
		ctx->next_frag_start = rs_dts;
	}
	ctx->next_frag_start += ctx->cdur.num;
	ctx->adjusted_next_frag_start = ctx->next_frag_start;
	ctx->fragment_started = GF_FALSE;
	if (ctx->noinit) {
		if (ctx->dst_pck) gf_filter_pck_discard(ctx->dst_pck);
		ctx->dst_pck = NULL;
		ctx->current_size = ctx->current_offset = 0;
		ctx->first_pck_sent = GF_FALSE;
	} else {
		mp4_mux_flush_seg(ctx, GF_TRUE, 0, 0, GF_TRUE);
	}
	assert(!ctx->dst_pck);
	if (ctx->styp && (strlen(ctx->styp)>=4)) {
		u32 styp_brand = GF_4CC(ctx->styp[0], ctx->styp[1], ctx->styp[2], ctx->styp[3]);
		u32 version = 0;
		char *sep = strchr(ctx->styp, '.');
		if (sep) version = atoi(sep+1);
		gf_isom_set_brand_info(ctx->file, styp_brand, version);
	}
	if (ctx->dash_mode==MP4MX_DASH_VOD) {
		if ((ctx->vodcache==MP4MX_VODCACHE_REPLACE) && !nb_segments && (!ctx->media_dur || !ctx->dash_dur.num) ) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[MP4Mux] Media duration unknown, cannot use replace mode of vodcache, using temp file for VoD storage\n"));
			ctx->vodcache = MP4MX_VODCACHE_ON;
			e = mp4mx_setup_dash_vod(ctx, NULL);
			if (e) return e;
		}
		if (ctx->vodcache==MP4MX_VODCACHE_REPLACE) {
			GF_BitStream *bs;
			u8 *output;
			char *msg;
			GF_FilterPacket *pck;
			u32 len;
			Bool exact_sidx = GF_TRUE;
			if (!nb_segments) {
				exact_sidx = GF_FALSE;
				nb_segments = (u32) ( ctx->media_dur * ctx->dash_dur.den / ctx->dash_dur.num);
				nb_segments ++;
				if (nb_segments>10)
					nb_segments += 10*nb_segments/100;
				else
					nb_segments ++;
			}
			ctx->sidx_max_size = 12 + (12 + 16) + 12 * nb_segments;
			if (ctx->ssix) {
				ctx->sidx_max_size += 12 + 4 + nb_segments * 12;
			}
			if (!exact_sidx) {
				ctx->sidx_max_size += 8;
				ctx->sidx_size_exact = GF_FALSE;
			} else {
				ctx->sidx_size_exact = GF_TRUE;
			}
			ctx->sidx_chunk_offset = (u32) (ctx->current_offset + ctx->current_size);
			pck = gf_filter_pck_new_alloc(ctx->opid, ctx->sidx_max_size, &output);
			if (!pck) return GF_OUT_OF_MEM;
			gf_filter_pck_set_framing(pck, GF_FALSE, GF_FALSE);
			bs = gf_bs_new(output, ctx->sidx_max_size, GF_BITSTREAM_WRITE);
			gf_bs_write_u32(bs, ctx->sidx_max_size);
			gf_bs_write_u32(bs, GF_ISOM_BOX_TYPE_FREE);
			msg = "GPAC " GPAC_VERSION" SIDX placeholder";
			len = (u32) strlen(msg);
			if (len+8>ctx->sidx_max_size) len = ctx->sidx_max_size - 8;
			gf_bs_write_data(bs, msg, len );
			gf_bs_del(bs);
			gf_filter_pck_send(pck);
			ctx->current_offset += ctx->sidx_max_size;
		} else if (ctx->vodcache==MP4MX_VODCACHE_ON) {
			ctx->store_output = GF_TRUE;
		} else {
			ctx->store_output = GF_FALSE;
			ctx->sidx_chunk_offset = (u32) (ctx->current_offset + ctx->current_size);
		}
		gf_isom_allocate_sidx(ctx->file, ctx->subs_sidx, ctx->chain_sidx, 0, NULL, NULL, NULL, ctx->ssix);
	}
	return GF_OK;
#else
	return GF_NOT_SUPPORTED;
#endif
}