ins_compl_stop(int c, int prev_mode, int retval)
{
    char_u	*ptr;
    int		want_cindent;
    if (compl_curr_match != NULL || compl_leader != NULL || c == Ctrl_E)
    {
	if (compl_curr_match != NULL && compl_used_match && c != Ctrl_E)
	    ptr = compl_curr_match->cp_str;
	else
	    ptr = NULL;
	ins_compl_fixRedoBufForLeader(ptr);
    }
    want_cindent = (get_can_cindent() && cindent_on());
    if (compl_cont_mode == CTRL_X_WHOLE_LINE)
    {
	if (want_cindent)
	{
	    do_c_expr_indent();
	    want_cindent = FALSE;	 
	}
    }
    else
    {
	int prev_col = curwin->w_cursor.col;
	if (prev_col > 0)
	    dec_cursor();
	if (!arrow_used && !ins_need_undo_get() && c != Ctrl_E)
	    insertchar(NUL, 0, -1);
	if (prev_col > 0
		&& ml_get_curline()[curwin->w_cursor.col] != NUL)
	    inc_cursor();
    }
    if ((c == Ctrl_Y || (compl_enter_selects
		    && (c == CAR || c == K_KENTER || c == NL)))
	    && pum_visible())
	retval = TRUE;
    if (c == Ctrl_E)
    {
	ins_compl_delete();
	if (compl_leader != NULL)
	    ins_bytes(compl_leader + get_compl_len());
	else if (compl_first_match != NULL)
	    ins_bytes(compl_orig_text + get_compl_len());
	retval = TRUE;
    }
    auto_format(FALSE, TRUE);
    ctrl_x_mode = prev_mode;
    ins_apply_autocmds(EVENT_COMPLETEDONEPRE);
    ins_compl_free();
    compl_started = FALSE;
    compl_matches = 0;
    if (!shortmess(SHM_COMPLETIONMENU))
	msg_clr_cmdline();	 
    ctrl_x_mode = CTRL_X_NORMAL;
    compl_enter_selects = FALSE;
    if (edit_submode != NULL)
    {
	edit_submode = NULL;
	showmode();
    }
#ifdef FEAT_CMDWIN
    if (c == Ctrl_C && cmdwin_type != 0)
	update_screen(0);
#endif
    if (want_cindent && in_cinkeys(KEY_COMPLETE, ' ', inindent(0)))
	do_c_expr_indent();
    ins_apply_autocmds(EVENT_COMPLETEDONE);
    return retval;
}