qf_jump_edit_buffer(
	qf_info_T	*qi,
	qfline_T	*qf_ptr,
	int		forceit,
	int		prev_winid,
	int		*opened_window)
{
    qf_list_T	*qfl = qf_get_curlist(qi);
    int		old_changedtick = qfl->qf_changedtick;
    qfltype_T	qfl_type = qfl->qfl_type;
    int		retval = OK;
    int		old_qf_curlist = qi->qf_curlist;
    int		save_qfid = qfl->qf_id;
    if (qf_ptr->qf_type == 1)
    {
	if (!can_abandon(curbuf, forceit))
	{
	    no_write_message();
	    return FAIL;
	}
	retval = do_ecmd(qf_ptr->qf_fnum, NULL, NULL, NULL, (linenr_T)1,
		ECMD_HIDE + ECMD_SET_HELP,
		prev_winid == curwin->w_id ? curwin : NULL);
    }
    else
	retval = buflist_getfile(qf_ptr->qf_fnum,
		(linenr_T)1, GETF_SETMARK | GETF_SWITCH, forceit);
    if (qfl_type == QFLT_LOCATION)
    {
	win_T	*wp = win_id2wp(prev_winid);
	if (wp == NULL && curwin->w_llist != qi)
	{
	    emsg(_(e_current_window_was_closed));
	    *opened_window = FALSE;
	    return NOTDONE;
	}
    }
    if (qfl_type == QFLT_QUICKFIX && !qflist_valid(NULL, save_qfid))
    {
	emsg(_(e_current_quickfix_list_was_changed));
	return NOTDONE;
    }
    if (old_qf_curlist != qi->qf_curlist
	    || old_changedtick != qfl->qf_changedtick
	    || !is_qf_entry_present(qfl, qf_ptr))
    {
	if (qfl_type == QFLT_QUICKFIX)
	    emsg(_(e_current_quickfix_list_was_changed));
	else
	    emsg(_(e_current_location_list_was_changed));
	return NOTDONE;
    }
    return retval;
}