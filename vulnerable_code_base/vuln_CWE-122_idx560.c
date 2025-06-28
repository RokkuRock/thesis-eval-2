GF_Err pcmreframe_process(GF_Filter *filter)
{
	GF_PCMReframeCtx *ctx = gf_filter_get_udta(filter);
	GF_FilterPacket *pck;
	u64 byte_offset;
	u8 *data;
	u32 pck_size;
	if (ctx->done) return GF_EOS;
	if (!ctx->is_playing && ctx->opid) return GF_OK;
	pck = gf_filter_pid_get_packet(ctx->ipid);
	if (!pck) {
		if (gf_filter_pid_is_eos(ctx->ipid) && !ctx->reverse_play) {
			if (ctx->out_pck) {
				gf_filter_pck_truncate(ctx->out_pck, ctx->nb_bytes_in_frame);
				gf_filter_pck_set_duration(ctx->out_pck, ctx->nb_bytes_in_frame/ctx->Bps/ctx->ch);
				pcmreframe_flush_packet(ctx);
			}
			if (ctx->opid)
				gf_filter_pid_set_eos(ctx->opid);
			return GF_EOS;
		}
		return GF_OK;
	}
	data = (char *) gf_filter_pck_get_data(pck, &pck_size);
	byte_offset = gf_filter_pck_get_byte_offset(pck);
	if (ctx->probe_wave==1) {
		Bool wav_ok = GF_TRUE;
		GF_BitStream *bs;
		if (ctx->probe_data) {
			ctx->probe_data = gf_realloc(ctx->probe_data, ctx->probe_data_size+pck_size);
			memcpy(ctx->probe_data + ctx->probe_data_size, data, pck_size);
			ctx->probe_data_size += pck_size;
			bs = gf_bs_new(ctx->probe_data, ctx->probe_data_size, GF_BITSTREAM_READ);
		} else {
			bs = gf_bs_new(data, pck_size, GF_BITSTREAM_READ);
		}
		u32 type = gf_bs_read_u32(bs);
		if (type!=GF_4CC('R', 'I', 'F', 'F')) {
			wav_ok = GF_FALSE;
		}
		gf_bs_read_u32(bs);
		u32 wtype = gf_bs_read_u32(bs);
		if (wtype!=GF_4CC('W', 'A', 'V', 'E')) {
			wav_ok = GF_FALSE;
		}
		while (gf_bs_available(bs)) {
			type = gf_bs_read_u32(bs);
			u32 csize = gf_bs_read_u32_le(bs);  
			if (type==GF_4CC('d', 'a', 't', 'a')) {
				break;
			}
			if (type!=GF_4CC('f', 'm', 't', ' ')) {
				gf_bs_skip_bytes(bs, csize);
				continue;
			}
			u16 atype = gf_bs_read_u16_le(bs);
			ctx->ch = gf_bs_read_u16_le(bs);
			ctx->sr = gf_bs_read_u32_le(bs);
			gf_bs_read_u32_le(bs);  
			gf_bs_read_u16_le(bs);  
			u32 bps = gf_bs_read_u16_le(bs);
			if (atype==3) {
				if (bps==32) {
					ctx->safmt = GF_AUDIO_FMT_FLT;
				} else {
					wav_ok = GF_FALSE;
				}
			} else if (atype==1) {
				if (bps==32) {
					ctx->safmt = GF_AUDIO_FMT_S32;
				} else if (bps==24) {
					ctx->safmt = GF_AUDIO_FMT_S24;
				} else if (bps==16) {
					ctx->safmt = GF_AUDIO_FMT_S16;
				} else if (bps==8) {
					ctx->safmt = GF_AUDIO_FMT_U8;
				} else {
					wav_ok = GF_FALSE;
				}
			}
		}
		if (gf_bs_is_overflow(bs)) {
			if (!ctx->probe_data) {
				ctx->probe_data = gf_malloc(pck_size);
				memcpy(ctx->probe_data, data, pck_size);
				ctx->probe_data_size = pck_size;
			}
			if (ctx->probe_data_size<=10000) {
				gf_filter_pid_drop_packet(ctx->ipid);
				return GF_OK;
			}
			GF_LOG(GF_LOG_WARNING, GF_LOG_MEDIA, ("[PCMReframe] Cannot find wave data chink afetr %d bytes, aborting\n", ctx->probe_data_size));
			wav_ok = GF_FALSE;
		}
		ctx->wav_hdr_size = (u32) gf_bs_get_position(bs);
		gf_bs_del(bs);
		if (!wav_ok) {
			gf_filter_pid_drop_packet(ctx->ipid);
			if (ctx->opid)
				gf_filter_pid_set_eos(ctx->opid);
			gf_filter_pid_set_discard(ctx->ipid, GF_TRUE);
			GF_LOG(GF_LOG_ERROR, GF_LOG_MEDIA, ("[PCMReframe] Invalid or unsupported WAVE header, aborting\n", ctx->probe_data_size));
			return GF_NON_COMPLIANT_BITSTREAM;
		}
		ctx->probe_wave = 2;
		pcmreframe_configure_pid(filter, ctx->ipid, GF_FALSE);
		if (ctx->probe_data) {
			pck_size = ctx->probe_data_size;
			data = ctx->probe_data;
		}
		pck_size -= ctx->wav_hdr_size;
		data+=ctx->wav_hdr_size;
		byte_offset = 0;
	}
	byte_offset+= ctx->wav_hdr_size;
	while (pck_size) {
		if (!ctx->out_pck) {
			ctx->out_pck = gf_filter_pck_new_alloc(ctx->opid, ctx->frame_size, &ctx->out_data);
			if (!ctx->out_pck) return GF_OUT_OF_MEM;
			gf_filter_pck_set_cts(ctx->out_pck, ctx->cts);
			gf_filter_pck_set_sap(ctx->out_pck, GF_FILTER_SAP_1);
			gf_filter_pck_set_duration(ctx->out_pck, ctx->framelen);
			gf_filter_pck_set_byte_offset(ctx->out_pck, byte_offset);
		}
		if (pck_size + ctx->nb_bytes_in_frame < ctx->frame_size) {
			memcpy(ctx->out_data + ctx->nb_bytes_in_frame, data, pck_size);
			ctx->nb_bytes_in_frame += pck_size;
			pck_size = 0;
		} else {
			u32 remain = ctx->frame_size - ctx->nb_bytes_in_frame;
			memcpy(ctx->out_data + ctx->nb_bytes_in_frame, data, remain);
			ctx->nb_bytes_in_frame = ctx->frame_size;
			pcmreframe_flush_packet(ctx);
			pck_size -= remain;
			data += remain;
			byte_offset += remain;
			ctx->out_pck = NULL;
			ctx->nb_bytes_in_frame = 0;
			if (ctx->reverse_play) {
				GF_FilterEvent fevt;
				if (!ctx->cts) {
					if (ctx->opid)
						gf_filter_pid_set_eos(ctx->opid);
					GF_FEVT_INIT(fevt, GF_FEVT_STOP, ctx->ipid);
					gf_filter_pid_send_event(ctx->ipid, &fevt);
					ctx->done = GF_TRUE;
					return GF_EOS;
				}
				ctx->cts -= ctx->framelen;
				ctx->filepos -= ctx->frame_size;
				gf_filter_pid_drop_packet(ctx->ipid);
				GF_FEVT_INIT(fevt, GF_FEVT_SOURCE_SEEK, ctx->ipid);
				fevt.seek.start_offset = ctx->filepos + ctx->wav_hdr_size;
				gf_filter_pid_send_event(ctx->ipid, &fevt);
				return GF_OK;
			}
			ctx->cts += ctx->framelen;
		}
	}
	gf_filter_pid_drop_packet(ctx->ipid);
	if (ctx->probe_data) {
		gf_free(ctx->probe_data);
		ctx->probe_data = NULL;
		ctx->probe_data_size = 0;
	}
	return GF_OK;
}