static GF_Err xviddec_configure_pid(GF_Filter *filter, GF_FilterPid *pid, Bool is_remove)
{
	const GF_PropertyValue *p;
	GF_M4VDecSpecInfo dsi;
	GF_Err e;
	Bool is_first = GF_FALSE;
#ifdef XVID_USE_OLD_API
	XVID_DEC_FRAME frame;
	XVID_DEC_PARAM par;
#else
	xvid_dec_frame_t frame;
	xvid_dec_create_t par;
#endif
	GF_XVIDCtx *ctx = gf_filter_get_udta(filter);
	if (is_remove) {
		if (ctx->opid) {
			gf_filter_pid_remove(ctx->opid);
			ctx->opid = NULL;
		}
		ctx->ipid = NULL;
		return GF_OK;
	}
	if (! gf_filter_pid_check_caps(pid))
		return GF_NOT_SUPPORTED;
	ctx->ipid = pid;
	if (!ctx->opid) {
		ctx->opid = gf_filter_pid_new(filter);
		gf_filter_pid_set_framing_mode(ctx->ipid, GF_TRUE);
		is_first = GF_TRUE;
	}
	gf_filter_pid_copy_properties(ctx->opid, ctx->ipid);
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_CODECID, &PROP_UINT(GF_CODECID_RAW) );
	p = gf_filter_pid_get_property(pid, GF_PROP_PID_DECODER_CONFIG);
	if (p && p->value.data.ptr && p->value.data.size) {
		u32 ex_crc = gf_crc_32(p->value.data.ptr, p->value.data.size);
		if (ctx->cfg_crc == ex_crc) return GF_OK;
		if (ctx->codec) xvid_decore(ctx->codec, XVID_DEC_DESTROY, NULL, NULL);
		ctx->codec = NULL;
		ctx->cfg_crc = ex_crc;
	} else if (!is_first) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CODEC, ("[XVID] Reconfiguring without DSI not yet supported\n"));
		return GF_NOT_SUPPORTED;
	} else {
		return GF_OK;
	}
	e = gf_m4v_get_config(p->value.data.ptr, p->value.data.size, &dsi);
	if (e) return e;
	if (!dsi.width || !dsi.height) return GF_NON_COMPLIANT_BITSTREAM;
	memset(&par, 0, sizeof(par));
	par.width = dsi.width;
	par.height = dsi.height;
	ctx->FPS = dsi.clock_rate;
	ctx->FPS /= 1000;
	if (!ctx->FPS) ctx->FPS = 30.0f;
	ctx->pixel_ar.num = dsi.par_num;
	ctx->pixel_ar.den = dsi.par_den;
#ifndef XVID_USE_OLD_API
	par.version = XVID_VERSION;
#endif
	if (xvid_decore(NULL, XVID_DEC_CREATE, &par, NULL) < 0) return GF_NON_COMPLIANT_BITSTREAM;
	ctx->width = par.width;
	ctx->height = par.height;
	ctx->codec = par.handle;
	memset(&frame, 0, sizeof(frame));
	frame.bitstream = (void *) p->value.data.ptr;
	frame.length = p->value.data.size;
#ifndef XVID_USE_OLD_API
	frame.version = XVID_VERSION;
	xvid_decore(ctx->codec, XVID_DEC_DECODE, &frame, NULL);
#else
	xvid_decore(ctx->codec, XVID_DEC_DECODE, &frame, NULL);
#endif
	ctx->first_frame = GF_TRUE;
	ctx->out_size = ctx->width * ctx->height * 3 / 2;
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_WIDTH, &PROP_UINT(ctx->width) );
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_HEIGHT, &PROP_UINT(ctx->height) );
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_STRIDE, &PROP_UINT(ctx->width) );
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PAR, &PROP_FRAC(ctx->pixel_ar) );
	gf_filter_pid_set_property(ctx->opid, GF_PROP_PID_PIXFMT, &PROP_UINT(GF_PIXEL_YUV) );
	return GF_OK;
}