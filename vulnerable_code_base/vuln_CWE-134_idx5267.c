int ecall_start(struct ecall *ecall, enum icall_call_type call_type,
		bool audio_cbr)
{
	int err;
	info("ecall(%p): start\n", ecall);
	if (!ecall)
		return EINVAL;
#ifdef ECALL_CBR_ALWAYS_ON
	audio_cbr = true;
#endif
	if (ecall->econn) {
		if (ECONN_PENDING_INCOMING == econn_current_state(ecall->econn)) {
			return ecall_answer(ecall, call_type, audio_cbr);
		}
		else {
			warning("ecall: start: already in progress (econn=%s)\n",
				econn_state_name(econn_current_state(ecall->econn)));
			return EALREADY;
		}
	}
#if 0
	if (ecall->turnc == 0) {
		warning("ecall: start: no TURN servers -- cannot start\n");
		return EINTR;
	}
#endif
	ecall->call_type = call_type;
	err = ecall_create_econn(ecall);
	if (err) {
		warning("ecall: start: create_econn failed: %m\n", err);
		return err;
	}
	econn_set_state(ecall_get_econn(ecall), ECONN_PENDING_OUTGOING);
	err = alloc_flow(ecall, ASYNC_OFFER, ecall->call_type, audio_cbr);
	if (err) {
		warning("ecall: start: alloc_flow failed: %m\n", err);
		goto out;
	}
	IFLOW_CALL(ecall->flow, set_audio_cbr, audio_cbr);
	if (ecall->props_local &&
	    (call_type == ICALL_CALL_TYPE_VIDEO
	     && ecall->vstate == ICALL_VIDEO_STATE_STARTED)) {
		const char *vstate_string = "true";
		int err2 = econn_props_update(ecall->props_local,
					      "videosend", vstate_string);
		if (err2) {
			warning("ecall(%p): econn_props_update(videosend)",
				" failed (%m)\n", ecall, err2);
		}
	}
	ecall->sdp.async = ASYNC_NONE;
	err = generate_offer(ecall);
	if (err) {
		warning("ecall(%p): start: generate_offer"
			" failed (%m)\n", ecall, err);
		goto out;
	}
	ecall->ts_started = tmr_jiffies();
	ecall->call_setup_time = -1;
 out:
	return err;
}