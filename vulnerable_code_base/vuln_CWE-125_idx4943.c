getvcol(
    win_T	*wp,
    pos_T	*pos,
    colnr_T	*start,
    colnr_T	*cursor,
    colnr_T	*end)
{
    colnr_T	vcol;
    char_u	*ptr;		 
    char_u	*posptr;	 
    char_u	*line;		 
    int		incr;
    int		head;
#ifdef FEAT_VARTABS
    int		*vts = wp->w_buffer->b_p_vts_array;
#endif
    int		ts = wp->w_buffer->b_p_ts;
    int		c;
    vcol = 0;
    line = ptr = ml_get_buf(wp->w_buffer, pos->lnum, FALSE);
    if (pos->col == MAXCOL)
	posptr = NULL;   
    else
    {
	if (*ptr == NUL)
	    pos->col = 0;
	posptr = ptr + pos->col;
	if (has_mbyte)
	    posptr -= (*mb_head_off)(line, posptr);
    }
    if ((!wp->w_p_list || wp->w_lcs_chars.tab1 != NUL)
#ifdef FEAT_LINEBREAK
	    && !wp->w_p_lbr && *get_showbreak_value(wp) == NUL && !wp->w_p_bri
#endif
       )
    {
	for (;;)
	{
	    head = 0;
	    c = *ptr;
	    if (c == NUL)
	    {
		incr = 1;	 
		break;
	    }
	    if (c == TAB)
#ifdef FEAT_VARTABS
		incr = tabstop_padding(vcol, ts, vts);
#else
		incr = ts - (vcol % ts);
#endif
	    else
	    {
		if (has_mbyte)
		{
		    if (enc_utf8 && c >= 0x80)
			incr = utf_ptr2cells(ptr);
		    else
			incr = g_chartab[c] & CT_CELL_MASK;
		    if (incr == 2 && wp->w_p_wrap && MB_BYTE2LEN(*ptr) > 1
			    && in_win_border(wp, vcol))
		    {
			++incr;
			head = 1;
		    }
		}
		else
		    incr = g_chartab[c] & CT_CELL_MASK;
	    }
	    if (posptr != NULL && ptr >= posptr)  
		break;
	    vcol += incr;
	    MB_PTR_ADV(ptr);
	}
    }
    else
    {
	for (;;)
	{
	    head = 0;
	    incr = win_lbr_chartabsize(wp, line, ptr, vcol, &head);
	    if (*ptr == NUL)
	    {
		incr = 1;	 
		break;
	    }
	    if (posptr != NULL && ptr >= posptr)  
		break;
	    vcol += incr;
	    MB_PTR_ADV(ptr);
	}
    }
    if (start != NULL)
	*start = vcol + head;
    if (end != NULL)
	*end = vcol + incr - 1;
    if (cursor != NULL)
    {
	if (*ptr == TAB
		&& (State & NORMAL)
		&& !wp->w_p_list
		&& !virtual_active()
		&& !(VIsual_active
				&& (*p_sel == 'e' || LTOREQ_POS(*pos, VIsual)))
		)
	    *cursor = vcol + incr - 1;	     
	else
	    *cursor = vcol + head;	     
    }
}