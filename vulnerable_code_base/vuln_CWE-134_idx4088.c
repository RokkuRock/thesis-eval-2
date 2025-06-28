int wcall_i_answer(struct wcall *wcall,
		   int call_type, int audio_cbr)
{
	int err = 0;
	bool cbr = audio_cbr != 0;
	if (!wcall) {
		warning("wcall; answer: no wcall\n");
		return EINVAL;
	}
	call_type = (call_type == WCALL_CALL_TYPE_FORCED_AUDIO) ?
		    WCALL_CALL_TYPE_NORMAL : call_type;
	info(APITAG "wcall(%p): answer calltype=%s\n",
	     wcall, wcall_call_type_name(call_type));
	if (wcall->disable_audio)
		wcall->disable_audio = false;
	if (!wcall->icall) {
		warning("wcall(%p): answer: no call object found\n", wcall);
		return ENOTSUP;
	}
	set_state(wcall, WCALL_STATE_ANSWERED);
	if (call_type == WCALL_CALL_TYPE_VIDEO) {
		ICALL_CALL(wcall->icall,
			   set_video_send_state,
			   ICALL_VIDEO_STATE_STARTED);
	}
	else {
		ICALL_CALL(wcall->icall,
			   set_video_send_state,
			   ICALL_VIDEO_STATE_STOPPED);
	}
	err = ICALL_CALLE(wcall->icall, answer,
			  call_type, cbr);
	return err;
}