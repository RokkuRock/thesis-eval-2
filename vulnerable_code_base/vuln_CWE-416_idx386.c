spell_move_to(
    win_T	*wp,
    int		dir,		 
    int		allwords,	 
    int		curline,
    hlf_T	*attrp)		 
{
    linenr_T	lnum;
    pos_T	found_pos;
    int		found_len = 0;
    char_u	*line;
    char_u	*p;
    char_u	*endp;
    hlf_T	attr;
    int		len;
#ifdef FEAT_SYN_HL
    int		has_syntax = syntax_present(wp);
#endif
    int		col;
    int		can_spell;
    char_u	*buf = NULL;
    int		buflen = 0;
    int		skip = 0;
    int		capcol = -1;
    int		found_one = FALSE;
    int		wrapped = FALSE;
    if (no_spell_checking(wp))
	return 0;
    lnum = wp->w_cursor.lnum;
    CLEAR_POS(&found_pos);
    while (!got_int)
    {
	line = ml_get_buf(wp->w_buffer, lnum, FALSE);
	len = (int)STRLEN(line);
	if (buflen < len + MAXWLEN + 2)
	{
	    vim_free(buf);
	    buflen = len + MAXWLEN + 2;
	    buf = alloc(buflen);
	    if (buf == NULL)
		break;
	}
	if (lnum == 1)
	    capcol = 0;
	if (capcol == 0)
	    capcol = getwhitecols(line);
	else if (curline && wp == curwin)
	{
	    col = getwhitecols(line);
	    if (check_need_cap(lnum, col))
		capcol = col;
	    line = ml_get_buf(wp->w_buffer, lnum, FALSE);
	}
	STRCPY(buf, line);
	if (lnum < wp->w_buffer->b_ml.ml_line_count)
	    spell_cat_line(buf + STRLEN(buf),
			  ml_get_buf(wp->w_buffer, lnum + 1, FALSE), MAXWLEN);
	p = buf + skip;
	endp = buf + len;
	while (p < endp)
	{
	    if (dir == BACKWARD
		    && lnum == wp->w_cursor.lnum
		    && !wrapped
		    && (colnr_T)(p - buf) >= wp->w_cursor.col)
		break;
	    attr = HLF_COUNT;
	    len = spell_check(wp, p, &attr, &capcol, FALSE);
	    if (attr != HLF_COUNT)
	    {
		if (allwords || attr == HLF_SPB)
		{
		    if (dir == BACKWARD
			    || lnum != wp->w_cursor.lnum
			    || (wrapped
				|| (colnr_T)(curline ? p - buf + len
						     : p - buf)
						  > wp->w_cursor.col))
		    {
#ifdef FEAT_SYN_HL
			if (has_syntax)
			{
			    col = (int)(p - buf);
			    (void)syn_get_id(wp, lnum, (colnr_T)col,
						    FALSE, &can_spell, FALSE);
			    if (!can_spell)
				attr = HLF_COUNT;
			}
			else
#endif
			    can_spell = TRUE;
			if (can_spell)
			{
			    found_one = TRUE;
			    found_pos.lnum = lnum;
			    found_pos.col = (int)(p - buf);
			    found_pos.coladd = 0;
			    if (dir == FORWARD)
			    {
				wp->w_cursor = found_pos;
				vim_free(buf);
				if (attrp != NULL)
				    *attrp = attr;
				return len;
			    }
			    else if (curline)
				found_pos.col += len;
			    found_len = len;
			}
		    }
		    else
			found_one = TRUE;
		}
	    }
	    p += len;
	    capcol -= len;
	}
	if (dir == BACKWARD && found_pos.lnum != 0)
	{
	    wp->w_cursor = found_pos;
	    vim_free(buf);
	    return found_len;
	}
	if (curline)
	    break;	 
	if (lnum == wp->w_cursor.lnum && wrapped)
	    break;
	if (dir == BACKWARD)
	{
	    if (lnum > 1)
		--lnum;
	    else if (!p_ws)
		break;	     
	    else
	    {
		lnum = wp->w_buffer->b_ml.ml_line_count;
		wrapped = TRUE;
		if (!shortmess(SHM_SEARCH))
		    give_warning((char_u *)_(top_bot_msg), TRUE);
	    }
	    capcol = -1;
	}
	else
	{
	    if (lnum < wp->w_buffer->b_ml.ml_line_count)
		++lnum;
	    else if (!p_ws)
		break;	     
	    else
	    {
		lnum = 1;
		wrapped = TRUE;
		if (!shortmess(SHM_SEARCH))
		    give_warning((char_u *)_(bot_top_msg), TRUE);
	    }
	    if (lnum == wp->w_cursor.lnum && !found_one)
		break;
	    if (attr == HLF_COUNT)
		skip = (int)(p - endp);
	    else
		skip = 0;
	    --capcol;
	    if (*skipwhite(line) == NUL)
		capcol = 0;
	}
	line_breakcheck();
    }
    vim_free(buf);
    return 0;
}