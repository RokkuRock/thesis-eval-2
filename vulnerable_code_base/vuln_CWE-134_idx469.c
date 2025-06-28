int ecall_answer(struct ecall *ecall, enum icall_call_type call_type,
		 bool audio_cbr)
{
	int err = 0;
	if (!ecall)
		return EINVAL;
#ifdef ECALL_CBR_ALWAYS_ON
	audio_cbr = true;
#endif
	info("ecall(%p): answer on pending econn %p call_type=%d\n", ecall, ecall->econn, call_type);
	if (!ecall->econn) {
		warning("ecall: answer: econn does not exist!\n");
		return ENOENT;
	}
	if (ECONN_PENDING_INCOMING != econn_current_state(ecall->econn)) {
		info("ecall(%p): answer: invalid state (%s)\n", ecall,
		     econn_state_name(econn_current_state(ecall->econn)));
		return EPROTO;
	}
	if (!ecall->flow) {
		warning("ecall: answer: no mediaflow\n");
		return EPROTO;
	}
	ecall->call_type = call_type;
	IFLOW_CALL(ecall->flow, set_call_type, call_type);
	ecall->audio_cbr = audio_cbr;
	IFLOW_CALL(ecall->flow, set_audio_cbr, audio_cbr);
#if 0
	if (ecall->props_local) {
		const char *vstate_string =
			call_type == ICALL_CALL_TYPE_VIDEO ? "true" : "false";
		int err2 = econn_props_update(ecall->props_local, "videosend", vstate_string);
		if (err2) {
			warning("ecall(%p): econn_props_update(videosend)",
				" failed (%m)\n", ecall, err2);
		}
	}
#endif
	err = generate_or_gather_answer(ecall, ecall->econn);
	if (err) {
		warning("ecall: answer: failed to gather_or_answer\n");
		goto out;
	}
	ecall->answered = true;
	ecall->audio_setup_time = -1;
	ecall->call_estab_time = -1;
	ecall->ts_answered = tmr_jiffies();
 out:
	return err;
}