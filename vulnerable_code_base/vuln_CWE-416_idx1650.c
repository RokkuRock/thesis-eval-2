win_enter_ext(
    win_T	*wp,
    int		undo_sync,
    int		curwin_invalid,
    int		trigger_new_autocmds,
    int		trigger_enter_autocmds,
    int		trigger_leave_autocmds)
{
    int		other_buffer = FALSE;
    if (wp == curwin && !curwin_invalid)	 
	return;
#ifdef FEAT_JOB_CHANNEL
    if (!curwin_invalid)
	leaving_window(curwin);
#endif
    if (!curwin_invalid && trigger_leave_autocmds)
    {
	if (wp->w_buffer != curbuf)
	{
	    apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, FALSE, curbuf);
	    other_buffer = TRUE;
	    if (!win_valid(wp))
		return;
	}
	apply_autocmds(EVENT_WINLEAVE, NULL, NULL, FALSE, curbuf);
	if (!win_valid(wp))
	    return;
#ifdef FEAT_EVAL
	if (aborting())
	    return;
#endif
    }
    if (undo_sync && curbuf != wp->w_buffer)
	u_sync(FALSE);
    update_topline();
    if (wp->w_buffer != curbuf)
	buf_copy_options(wp->w_buffer, BCO_ENTER | BCO_NOHELP);
    if (!curwin_invalid)
    {
	prevwin = curwin;	 
	curwin->w_redr_status = TRUE;
    }
    curwin = wp;
    curbuf = wp->w_buffer;
    check_cursor();
    if (!virtual_active())
	curwin->w_cursor.coladd = 0;
    changed_line_abv_curs();	 
    if (curwin->w_localdir != NULL || curtab->tp_localdir != NULL)
    {
	char_u	*dirname;
	if (globaldir == NULL)
	{
	    char_u	cwd[MAXPATHL];
	    if (mch_dirname(cwd, MAXPATHL) == OK)
		globaldir = vim_strsave(cwd);
	}
	if (curwin->w_localdir != NULL)
	    dirname = curwin->w_localdir;
	else
	    dirname = curtab->tp_localdir;
	if (mch_chdir((char *)dirname) == 0)
	    shorten_fnames(TRUE);
    }
    else if (globaldir != NULL)
    {
	vim_ignored = mch_chdir((char *)globaldir);
	VIM_CLEAR(globaldir);
	shorten_fnames(TRUE);
    }
#ifdef FEAT_JOB_CHANNEL
    entering_window(curwin);
#endif
    if (trigger_new_autocmds)
	apply_autocmds(EVENT_WINNEW, NULL, NULL, FALSE, curbuf);
    if (trigger_enter_autocmds)
    {
	apply_autocmds(EVENT_WINENTER, NULL, NULL, FALSE, curbuf);
	if (other_buffer)
	    apply_autocmds(EVENT_BUFENTER, NULL, NULL, FALSE, curbuf);
    }
#ifdef FEAT_TITLE
    maketitle();
#endif
    curwin->w_redr_status = TRUE;
#ifdef FEAT_TERMINAL
    if (bt_terminal(wp->w_buffer))
	redraw_mode = TRUE;
#endif
    redraw_tabline = TRUE;
    if (restart_edit)
	redraw_later(VALID);	 
    if (curwin->w_height < p_wh && !curwin->w_p_wfh
#ifdef FEAT_TEXT_PROP
	    && !popup_is_popup(curwin)
#endif
	    )
	win_setheight((int)p_wh);
    else if (curwin->w_height == 0)
	win_setheight(1);
    if (curwin->w_width < p_wiw && !curwin->w_p_wfw)
	win_setwidth((int)p_wiw);
    setmouse();			 
    DO_AUTOCHDIR;
}