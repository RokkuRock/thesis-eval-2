GF_Err mhas_dmx_process(GF_Filter *filter)
{
	GF_MHASDmxCtx *ctx = gf_filter_get_udta(filter);
	GF_FilterPacket *in_pck;
	u8 *output;
	u8 *start;
	Bool final_flush=GF_FALSE;
	u32 pck_size, remain, prev_pck_size;
	u64 cts = GF_FILTER_NO_TS;
	u32 au_start = 0;
	u32 consumed = 0;
	u32 nb_trunc_samples = 0;
	Bool trunc_from_begin = 0;
	Bool has_cfg = 0;
	if (!ctx->duration.num)
		mhas_dmx_check_dur(filter, ctx);
	if (ctx->opid && !ctx->is_playing)
		return GF_OK;
	in_pck = gf_filter_pid_get_packet(ctx->ipid);
	if (!in_pck) {
		if (gf_filter_pid_is_eos(ctx->ipid)) {
			if (!ctx->mhas_buffer_size) {
				if (ctx->opid)
					gf_filter_pid_set_eos(ctx->opid);
				if (ctx->src_pck) gf_filter_pck_unref(ctx->src_pck);
				ctx->src_pck = NULL;
				return GF_EOS;
			}
			final_flush = GF_TRUE;
		} else if (!ctx->resume_from) {
			return GF_OK;
		}
	}
	prev_pck_size = ctx->mhas_buffer_size;
	if (ctx->resume_from)
		in_pck = NULL;
	if (in_pck) {
		u8 *data = (u8 *) gf_filter_pck_get_data(in_pck, &pck_size);
		if (ctx->byte_offset != GF_FILTER_NO_BO) {
			u64 byte_offset = gf_filter_pck_get_byte_offset(in_pck);
			if (!ctx->mhas_buffer_size) {
				ctx->byte_offset = byte_offset;
			} else if (ctx->byte_offset + ctx->mhas_buffer_size != byte_offset) {
				ctx->byte_offset = GF_FILTER_NO_BO;
				if ((byte_offset != GF_FILTER_NO_BO) && (byte_offset>ctx->mhas_buffer_size) ) {
					ctx->byte_offset = byte_offset - ctx->mhas_buffer_size;
				}
			}
		}
		if (ctx->mhas_buffer_size + pck_size > ctx->mhas_buffer_alloc) {
			ctx->mhas_buffer_alloc = ctx->mhas_buffer_size + pck_size;
			ctx->mhas_buffer = gf_realloc(ctx->mhas_buffer, ctx->mhas_buffer_alloc);
		}
		memcpy(ctx->mhas_buffer + ctx->mhas_buffer_size, data, pck_size);
		ctx->mhas_buffer_size += pck_size;
	}
	if (ctx->timescale && in_pck) {
		cts = gf_filter_pck_get_cts(in_pck);
		if (!ctx->cts && (cts != GF_FILTER_NO_TS))
			ctx->cts = cts;
	}
	if (cts == GF_FILTER_NO_TS) {
		prev_pck_size = 0;
	}
	remain = ctx->mhas_buffer_size;
	start = ctx->mhas_buffer;
	if (ctx->resume_from) {
		start += ctx->resume_from - 1;
		remain -= ctx->resume_from - 1;
		ctx->resume_from = 0;
	}
	while (ctx->nosync && (remain>3)) {
		u8 *hdr_start = memchr(start, 0xC0, remain);
		if (!hdr_start) {
			remain=0;
			break;
		}
		if ((hdr_start[1]==0x01) && (hdr_start[2]==0xA5)) {
			GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] Sync found !\n"));
			ctx->nosync = GF_FALSE;
			break;
		}
		GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] not sync, skipping byte\n"));
		start++;
		remain--;
	}
	if (ctx->nosync)
		goto skip;
	gf_bs_reassign_buffer(ctx->bs, start, remain);
	ctx->buffer_too_small = GF_FALSE;
	while (remain > consumed) {
		u32 pay_start, parse_end, mhas_size, mhas_label;
		Bool mhas_sap = 0;
		u32 mhas_type;
		if (!ctx->is_playing && ctx->opid) {
			ctx->resume_from = 1;
			consumed = 0;
			break;
		}
		mhas_type = (u32) gf_mpegh_escaped_value(ctx->bs, 3, 8, 8);
		mhas_label = (u32) gf_mpegh_escaped_value(ctx->bs, 2, 8, 32);
		mhas_size = (u32) gf_mpegh_escaped_value(ctx->bs, 11, 24, 24);
		if (ctx->buffer_too_small)
			break;
		if (mhas_type>18) {
			ctx->nb_unknown_pck++;
			if (ctx->nb_unknown_pck > ctx->pcksync) {
				GF_LOG(ctx->is_sync ? GF_LOG_WARNING : GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] %d packets of unknown type, considering sync was lost\n"));
				ctx->is_sync = GF_FALSE;
				consumed = 0;
				ctx->nosync = GF_TRUE;
				ctx->nb_unknown_pck = 0;
				break;
			}
		} else if (!mhas_size) {
			GF_LOG(ctx->is_sync ? GF_LOG_WARNING : GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] MHAS packet with 0 payload size, considering sync was lost\n"));
			ctx->is_sync = GF_FALSE;
			consumed = 0;
			ctx->nosync = GF_TRUE;
			ctx->nb_unknown_pck = 0;
			break;
		}
		pay_start = (u32) gf_bs_get_position(ctx->bs);
		if (ctx->buffer_too_small) break;
		if (mhas_size > gf_bs_available(ctx->bs)) {
			GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] incomplete packet type %d %s label "LLU" size "LLU" - keeping in buffer\n", mhas_type, mhas_pck_name(mhas_type), mhas_label, mhas_size));
			break;
		}
		ctx->is_sync = GF_TRUE;
		if (mhas_type==2) {
			mhas_sap = gf_bs_peek_bits(ctx->bs, 1, 0);
			ctx->nb_unknown_pck = 0;
		}
		else if (mhas_type==1) {
			s32 CICPspeakerLayoutIdx = -1;
			s32 numSpeakers = -1;
			u32 sr = 0;
			u32 frame_len;
			u32 pl = gf_bs_read_u8(ctx->bs);
			u32 idx = gf_bs_read_int(ctx->bs, 5);
			if (idx==0x1f)
				sr = gf_bs_read_int(ctx->bs, 24);
			else if (sr < nb_usac_sr) {
				sr = USACSampleRates[idx];
			}
			ctx->nb_unknown_pck = 0;
			idx = gf_bs_read_int(ctx->bs, 3);
			if ((idx==0) || (idx==2) ) frame_len = 768;
			else frame_len = 1024;
			gf_bs_read_int(ctx->bs, 1);
			gf_bs_read_int(ctx->bs, 1);
			u32 speakerLayoutType = gf_bs_read_int(ctx->bs, 2);
			if (speakerLayoutType == 0) {
				CICPspeakerLayoutIdx = gf_bs_read_int(ctx->bs, 6);
			} else {
				numSpeakers = (s32) gf_mpegh_escaped_value(ctx->bs, 5, 8, 16) + 1;
			}
			mhas_dmx_check_pid(filter, ctx, pl, sr, frame_len, CICPspeakerLayoutIdx, numSpeakers, start + pay_start, (u32) mhas_size);
			has_cfg = GF_TRUE;
		}
		else if (mhas_type==17) {
			Bool isActive = gf_bs_read_int(ctx->bs, 1);
			 gf_bs_read_int(ctx->bs, 1);
			trunc_from_begin = gf_bs_read_int(ctx->bs, 1);
			nb_trunc_samples = gf_bs_read_int(ctx->bs, 13);
			if (!isActive) {
				nb_trunc_samples = 0;
			}
		}
		else if ((mhas_type==6) || (mhas_type==7)) {
			ctx->nb_unknown_pck = 0;
		}
#if 0
		else if (mhas_type==8) {
			u8 marker_type = gf_bs_read_u8(ctx->bs);
			if (marker_type==0x01) {}
			else if (marker_type==0x02) {
				has_marker = GF_TRUE;
			}
		}
#endif
		gf_bs_align(ctx->bs);
		parse_end = (u32) gf_bs_get_position(ctx->bs) - pay_start;
		gf_bs_skip_bytes(ctx->bs, mhas_size - parse_end);
		GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] MHAS Packet type %d %s label "LLU" size "LLU"\n", mhas_type, mhas_pck_name(mhas_type), mhas_label, mhas_size));
		if (ctx->timescale && !prev_pck_size && (cts != GF_FILTER_NO_TS) ) {
			ctx->cts = cts;
			cts = GF_FILTER_NO_TS;
		}
		if ((mhas_type==2) && ctx->opid) {
			GF_FilterPacket *dst;
			u64 pck_dur = ctx->frame_len;
			u32 au_size;
			if (ctx->mpha) {
				au_start = pay_start;
				au_size = mhas_size;
			} else {
				au_size = (u32) gf_bs_get_position(ctx->bs) - au_start;
			}
			if (nb_trunc_samples) {
				if (trunc_from_begin) {
					if (!ctx->nb_frames) {
						s64 offset = trunc_from_begin;
						if (ctx->timescale) {
							offset *= ctx->timescale;
							offset /= ctx->sample_rate;
						}
						gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_DELAY , &PROP_LONGSINT( -offset));
					}
				} else {
					pck_dur -= nb_trunc_samples;
				}
				nb_trunc_samples = 0;
			}
			if (ctx->timescale) {
				pck_dur *= ctx->timescale;
				pck_dur /= ctx->sample_rate;
			}
			dst = gf_filter_pck_new_alloc(ctx->opid, au_size, &output);
			if (!dst) break;
			if (ctx->src_pck) gf_filter_pck_merge_properties(ctx->src_pck, dst);
			memcpy(output, start + au_start, au_size);
			if (!has_cfg)
				mhas_sap = 0;
			if (mhas_sap) {
				gf_filter_pck_set_sap(dst, GF_FILTER_SAP_1);
			}
			gf_filter_pck_set_dts(dst, ctx->cts);
			gf_filter_pck_set_cts(dst, ctx->cts);
			gf_filter_pck_set_duration(dst, (u32) pck_dur);
			if (ctx->byte_offset != GF_FILTER_NO_BO) {
				u64 offset = (u64) (start - ctx->mhas_buffer);
				offset += ctx->byte_offset + au_start;
				gf_filter_pck_set_byte_offset(dst, offset);
			}
 			GF_LOG(GF_LOG_DEBUG, GF_LOG_MEDIA, ("[MHASDmx] Send AU CTS "LLU" size %d dur %d sap %d\n", ctx->cts, au_size, (u32) pck_dur, mhas_sap));
			gf_filter_pck_send(dst);
			au_start += au_size;
			consumed = au_start;
			ctx->nb_frames ++;
			mhas_dmx_update_cts(ctx);
			has_cfg = 0;
			if (prev_pck_size) {
				u64 next_pos = (u64) (start + au_start - ctx->mhas_buffer);
				if (prev_pck_size <= next_pos) {
					prev_pck_size = 0;
					if (ctx->src_pck) gf_filter_pck_unref(ctx->src_pck);
					ctx->src_pck = in_pck;
					if (in_pck)
						gf_filter_pck_ref_props(&ctx->src_pck);
					if (ctx->timescale && (cts != GF_FILTER_NO_TS) ) {
						ctx->cts = cts;
						cts = GF_FILTER_NO_TS;
					}
				}
			}
			if (remain==consumed)
				break;
			if (gf_filter_pid_would_block(ctx->opid)) {
				ctx->resume_from = 1;
				final_flush = GF_FALSE;
				break;
			}
		}
	}
	if (consumed) {
		assert(remain>=consumed);
		remain -= consumed;
		start += consumed;
	}
skip:
	if (remain < ctx->mhas_buffer_size) {
		memmove(ctx->mhas_buffer, start, remain);
		if (ctx->byte_offset != GF_FILTER_NO_BO)
			ctx->byte_offset += ctx->mhas_buffer_size - remain;
	}
	ctx->mhas_buffer_size = remain;
	if (final_flush)
		ctx->mhas_buffer_size = 0;
	if (!ctx->mhas_buffer_size) {
		if (ctx->src_pck) gf_filter_pck_unref(ctx->src_pck);
		ctx->src_pck = NULL;
	}
	if (in_pck)
		gf_filter_pid_drop_packet(ctx->ipid);
	return GF_OK;
}