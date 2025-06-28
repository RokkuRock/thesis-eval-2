jpc_ms_t *jpc_getms(jas_stream_t *in, jpc_cstate_t *cstate)
{
	jpc_ms_t *ms;
	jpc_mstabent_t *mstabent;
	jas_stream_t *tmpstream;
	if (!(ms = jpc_ms_create(0))) {
		return 0;
	}
	if (jpc_getuint16(in, &ms->id) || ms->id < JPC_MS_MIN ||
	  ms->id > JPC_MS_MAX) {
		jpc_ms_destroy(ms);
		return 0;
	}
	mstabent = jpc_mstab_lookup(ms->id);
	ms->ops = &mstabent->ops;
	if (JPC_MS_HASPARMS(ms->id)) {
		if (jpc_getuint16(in, &ms->len) || ms->len < 3) {
			jpc_ms_destroy(ms);
			return 0;
		}
		ms->len -= 2;
		if (!(tmpstream = jas_stream_memopen(0, 0))) {
			jpc_ms_destroy(ms);
			return 0;
		}
		if (jas_stream_copy(tmpstream, in, ms->len) ||
		  jas_stream_seek(tmpstream, 0, SEEK_SET) < 0) {
			jas_stream_close(tmpstream);
			jpc_ms_destroy(ms);
			return 0;
		}
		if ((*ms->ops->getparms)(ms, cstate, tmpstream)) {
			ms->ops = 0;
			jpc_ms_destroy(ms);
			jas_stream_close(tmpstream);
			return 0;
		}
		if (jas_getdbglevel() > 0) {
			jpc_ms_dump(ms, stderr);
		}
		if (JAS_CAST(ulong, jas_stream_tell(tmpstream)) != ms->len) {
			jas_eprintf(
			  "warning: trailing garbage in marker segment (%ld bytes)\n",
			  ms->len - jas_stream_tell(tmpstream));
		}
		jas_stream_close(tmpstream);
	} else {
		ms->len = 0;
		if (jas_getdbglevel() > 0) {
			jpc_ms_dump(ms, stderr);
		}
	}
	if (ms->id == JPC_MS_SIZ) {
		cstate->numcomps = ms->parms.siz.numcomps;
	}
	return ms;
}