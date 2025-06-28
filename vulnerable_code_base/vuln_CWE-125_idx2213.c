static void mhas_dmx_check_dur(GF_Filter *filter, GF_MHASDmxCtx *ctx)
{
	GF_Fraction64 duration;
	FILE *stream;
	GF_BitStream *bs;
	u32 frame_len, cur_dur;
	Bool mhas_sap;
	u64 mhas_last_cfg, rate;
	const GF_PropertyValue *p;
	if (!ctx->opid || ctx->timescale || ctx->file_loaded) return;
	if (ctx->index<=0) {
		ctx->file_loaded = GF_TRUE;
		return;
	}
	p = gf_filter_pid_get_property(ctx->ipid, GF_PROP_PID_FILEPATH);
	if (!p || !p->value.string || !strncmp(p->value.string, "gmem://", 7)) {
		ctx->is_file = GF_FALSE;
		ctx->file_loaded = GF_TRUE;
		return;
	}
	ctx->is_file = GF_TRUE;
	stream = gf_fopen_ex(p->value.string, NULL, "rb", GF_TRUE);
	if (!stream) {
		if (gf_fileio_is_main_thread(p->value.string))
			ctx->file_loaded = GF_TRUE;
		return;
	}
	ctx->index_size = 0;
	bs = gf_bs_from_file(stream, GF_BITSTREAM_READ);
	duration.num = duration.den = 0;
	frame_len = cur_dur = 0;
	mhas_last_cfg = 0;
	while (gf_bs_available(bs)) {
		u32 sync_code = gf_bs_peek_bits(bs, 24, 0);
		if (sync_code == 0xC001A5) {
			break;
		}
		gf_bs_skip_bytes(bs, 1);
	}
	while (gf_bs_available(bs)) {
		u64 mhas_pck_start, pay_start, parse_end, mhas_size;
		u32 mhas_type;
		mhas_pck_start = gf_bs_get_position(bs);
		mhas_type = (u32) gf_mpegh_escaped_value(bs, 3, 8, 8);
		 gf_mpegh_escaped_value(bs, 2, 8, 32);
		mhas_size = gf_mpegh_escaped_value(bs, 11, 24, 24);
		pay_start = (u32) gf_bs_get_position(bs);
		if (!gf_bs_available(bs) ) break;
		if (mhas_size > gf_bs_available(bs)) break;
		mhas_sap = 0;
		if (mhas_type==2) {
			mhas_sap = gf_bs_read_int(bs, 1);
			if (!mhas_last_cfg) mhas_sap = 0;
		} else if (mhas_type==1) {
			u32 sr = 0;
			 gf_bs_read_u8(bs);
			u32 idx = gf_bs_read_int(bs, 5);
			if (idx==0x1f)
				duration.den = gf_bs_read_int(bs, 24);
			else if (sr < nb_usac_sr) {
				duration.den = USACSampleRates[idx];
			}
			idx = gf_bs_read_int(bs, 3);
			if ((idx==0) || (idx==2) ) frame_len = 768;
			else frame_len = 1024;
			mhas_last_cfg = mhas_pck_start;
		}
		else if (mhas_type==17) {
			Bool isActive = gf_bs_read_int(bs, 1);
			 gf_bs_read_int(bs, 1);
			Bool trunc_from_begin = gf_bs_read_int(bs, 1);
			u32 nb_trunc_samples = gf_bs_read_int(bs, 13);
			if (isActive && !trunc_from_begin) {
				duration.num -= nb_trunc_samples;
			}
		}
		gf_bs_align(bs);
		parse_end = (u32) gf_bs_get_position(bs) - pay_start;
		gf_bs_skip_bytes(bs, mhas_size - parse_end);
		if (mhas_sap && duration.den && (cur_dur >= ctx->index * duration.den) ) {
			if (!ctx->index_alloc_size) ctx->index_alloc_size = 10;
			else if (ctx->index_alloc_size == ctx->index_size) ctx->index_alloc_size *= 2;
			ctx->indexes = gf_realloc(ctx->indexes, sizeof(MHASIdx)*ctx->index_alloc_size);
			ctx->indexes[ctx->index_size].pos = mhas_last_cfg;
			ctx->indexes[ctx->index_size].duration = ((Double) duration.num) / duration.den;
			ctx->index_size ++;
			cur_dur = 0;
		}
		if (mhas_type==2) {
			duration.num += frame_len;
			cur_dur += frame_len;
			mhas_last_cfg = 0;
		}
	}
	rate = gf_bs_get_position(bs);
	gf_bs_del(bs);
	gf_fclose(stream);
	if (!ctx->duration.num || (ctx->duration.num  * duration.den != duration.num * ctx->duration.den)) {
		ctx->duration = duration;
		gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_DURATION, & PROP_FRAC64(ctx->duration));
		if (duration.num && !gf_sys_is_test_mode() ) {
			rate *= 8 * ctx->duration.den;
			rate /= ctx->duration.num;
			ctx->bitrate = (u32) rate;
		}
	}
	p = gf_filter_pid_get_property(ctx->ipid, GF_PROP_PID_FILE_CACHED);
	if (p && p->value.boolean) ctx->file_loaded = GF_TRUE;
}