qf_jump_open_window(
	qf_info_T	*qi,
	qfline_T	*qf_ptr,
	int		newwin,
	int		*opened_window)
{
    qf_list_T	*qfl = qf_get_curlist(qi);
    int		old_changedtick = qfl->qf_changedtick;
    int		old_qf_curlist = qi->qf_curlist;
    qfltype_T	qfl_type = qfl->qfl_type;
    if (qf_ptr->qf_type == 1 && (!bt_help(curwin->w_buffer)
						      || cmdmod.cmod_tab != 0))
	if (jump_to_help_window(qi, newwin, opened_window) == FAIL)
	    return FAIL;
    if (old_qf_curlist != qi->qf_curlist
	    || old_changedtick != qfl->qf_changedtick
	    || !is_qf_entry_present(qfl, qf_ptr))
    {
	if (qfl_type == QFLT_QUICKFIX)
	    emsg(_(e_current_quickfix_list_was_changed));
	else
	    emsg(_(e_current_location_list_was_changed));
	return FAIL;
    }
    if (bt_quickfix(curbuf) && !*opened_window)
    {
	if (qf_ptr->qf_fnum == 0)
	    return NOTDONE;
	if (qf_jump_to_usable_window(qf_ptr->qf_fnum, newwin,
						opened_window) == FAIL)
	    return FAIL;
    }
    if (old_qf_curlist != qi->qf_curlist
	    || old_changedtick != qfl->qf_changedtick
	    || !is_qf_entry_present(qfl, qf_ptr))
    {
	if (qfl_type == QFLT_QUICKFIX)
	    emsg(_(e_current_quickfix_list_was_changed));
	else
	    emsg(_(e_current_location_list_was_changed));
	return FAIL;
    }
    return OK;
}