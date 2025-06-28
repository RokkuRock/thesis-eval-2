void gru_check_context_placement(struct gru_thread_state *gts)
{
	struct gru_state *gru;
	gru = gts->ts_gru;
	if (!gru || gts->ts_tgid_owner != current->tgid)
		return;
	if (!gru_check_chiplet_assignment(gru, gts)) {
		STAT(check_context_unload);
		gru_unload_context(gts, 1);
	} else if (gru_retarget_intr(gts)) {
		STAT(check_context_retarget_intr);
	}
}