stop_insert(
    pos_T	*end_insert_pos,
    int		esc,			 
    int		nomove)			 
{
    int		cc;
    char_u	*ptr;
    stop_redo_ins();
    replace_flush();		 
    ptr = get_inserted();
    if (did_restart_edit == 0 || (ptr != NULL
				       && (int)STRLEN(ptr) > new_insert_skip))
    {
	vim_free(last_insert);
	last_insert = ptr;
	last_insert_skip = new_insert_skip;
    }
    else
	vim_free(ptr);
    if (!arrow_used && end_insert_pos != NULL)
    {
	if (!ins_need_undo && has_format_option(FO_AUTO))
	{
	    pos_T   tpos = curwin->w_cursor;
	    cc = 'x';
	    if (curwin->w_cursor.col > 0 && gchar_cursor() == NUL)
	    {
		dec_cursor();
		cc = gchar_cursor();
		if (!VIM_ISWHITE(cc))
		    curwin->w_cursor = tpos;
	    }
	    auto_format(TRUE, FALSE);
	    if (VIM_ISWHITE(cc))
	    {
		if (gchar_cursor() != NUL)
		    inc_cursor();
		if (gchar_cursor() == NUL
			&& curwin->w_cursor.lnum == tpos.lnum
			&& curwin->w_cursor.col == tpos.col)
		    curwin->w_cursor.coladd = tpos.coladd;
	    }
	}
	check_auto_format(TRUE);
	if (!nomove && did_ai && (esc || (vim_strchr(p_cpo, CPO_INDENT) == NULL
			&& curwin->w_cursor.lnum != end_insert_pos->lnum))
		&& end_insert_pos->lnum <= curbuf->b_ml.ml_line_count)
	{
	    pos_T	tpos = curwin->w_cursor;
	    curwin->w_cursor = *end_insert_pos;
	    check_cursor_col();   
	    for (;;)
	    {
		if (gchar_cursor() == NUL && curwin->w_cursor.col > 0)
		    --curwin->w_cursor.col;
		cc = gchar_cursor();
		if (!VIM_ISWHITE(cc))
		    break;
		if (del_char(TRUE) == FAIL)
		    break;   
	    }
	    if (curwin->w_cursor.lnum != tpos.lnum)
		curwin->w_cursor = tpos;
	    else
	    {
		tpos = curwin->w_cursor;
		tpos.col++;
		if (cc != NUL && gchar_pos(&tpos) == NUL)
		    ++curwin->w_cursor.col;	 
	    }
	    if (VIsual_active && VIsual.lnum == curwin->w_cursor.lnum)
	    {
		int len = (int)STRLEN(ml_get_curline());
		if (VIsual.col > len)
		{
		    VIsual.col = len;
		    VIsual.coladd = 0;
		}
	    }
	}
    }
    did_ai = FALSE;
#ifdef FEAT_SMARTINDENT
    did_si = FALSE;
    can_si = FALSE;
    can_si_back = FALSE;
#endif
    if (end_insert_pos != NULL)
    {
	curbuf->b_op_start = Insstart;
	curbuf->b_op_start_orig = Insstart_orig;
	curbuf->b_op_end = *end_insert_pos;
    }
}