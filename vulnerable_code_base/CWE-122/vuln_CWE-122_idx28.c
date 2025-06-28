buflist_getfile(
    int		n,
    linenr_T	lnum,
    int		options,
    int		forceit)
{
    buf_T	*buf;
    win_T	*wp = NULL;
    pos_T	*fpos;
    colnr_T	col;
    buf = buflist_findnr(n);
    if (buf == NULL)
    {
	if ((options & GETF_ALT) && n == 0)
	    emsg(_(e_no_alternate_file));
	else
	    semsg(_(e_buffer_nr_not_found), n);
	return FAIL;
    }
    if (buf == curbuf)
	return OK;
    if (text_locked())
    {
	text_locked_msg();
	return FAIL;
    }
    if (curbuf_locked())
	return FAIL;
    if (lnum == 0)
    {
	fpos = buflist_findfpos(buf);
	lnum = fpos->lnum;
	col = fpos->col;
    }
    else
	col = 0;
    if (options & GETF_SWITCH)
    {
	if (swb_flags & SWB_USEOPEN)
	    wp = buf_jump_open_win(buf);
	if (wp == NULL && (swb_flags & SWB_USETAB))
	    wp = buf_jump_open_tab(buf);
	if (wp == NULL && (swb_flags & (SWB_VSPLIT | SWB_SPLIT | SWB_NEWTAB))
							       && !BUFEMPTY())
	{
	    if (swb_flags & SWB_NEWTAB)
		tabpage_new();
	    else if (win_split(0, (swb_flags & SWB_VSPLIT) ? WSP_VERT : 0)
								      == FAIL)
		return FAIL;
	    RESET_BINDING(curwin);
	}
    }
    ++RedrawingDisabled;
    if (GETFILE_SUCCESS(getfile(buf->b_fnum, NULL, NULL,
				     (options & GETF_SETMARK), lnum, forceit)))
    {
	--RedrawingDisabled;
	if (!p_sol && col != 0)
	{
	    curwin->w_cursor.col = col;
	    check_cursor_col();
	    curwin->w_cursor.coladd = 0;
	    curwin->w_set_curswant = TRUE;
	}
	return OK;
    }
    --RedrawingDisabled;
    return FAIL;
}