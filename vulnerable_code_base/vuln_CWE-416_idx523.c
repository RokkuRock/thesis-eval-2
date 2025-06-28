nv_replace(cmdarg_T *cap)
{
    char_u	*ptr;
    int		had_ctrl_v;
    long	n;
    if (checkclearop(cap->oap))
	return;
#ifdef FEAT_JOB_CHANNEL
    if (bt_prompt(curbuf) && !prompt_curpos_editable())
    {
	clearopbeep(cap->oap);
	return;
    }
#endif
    if (cap->nchar == Ctrl_V)
    {
	had_ctrl_v = Ctrl_V;
	cap->nchar = get_literal(FALSE);
	if (cap->nchar > DEL)
	    had_ctrl_v = NUL;
    }
    else
	had_ctrl_v = NUL;
    if (IS_SPECIAL(cap->nchar))
    {
	clearopbeep(cap->oap);
	return;
    }
    if (VIsual_active)
    {
	if (got_int)
	    reset_VIsual();
	if (had_ctrl_v)
	{
	    if (cap->nchar == CAR)
		cap->nchar = REPLACE_CR_NCHAR;
	    else if (cap->nchar == NL)
		cap->nchar = REPLACE_NL_NCHAR;
	}
	nv_operator(cap);
	return;
    }
    if (virtual_active())
    {
	if (u_save_cursor() == FAIL)
	    return;
	if (gchar_cursor() == NUL)
	{
	    coladvance_force((colnr_T)(getviscol() + cap->count1));
	    curwin->w_cursor.col -= cap->count1;
	}
	else if (gchar_cursor() == TAB)
	    coladvance_force(getviscol());
    }
    ptr = ml_get_cursor();
    if (STRLEN(ptr) < (unsigned)cap->count1
	    || (has_mbyte && mb_charlen(ptr) < cap->count1))
    {
	clearopbeep(cap->oap);
	return;
    }
    if (had_ctrl_v != Ctrl_V && cap->nchar == '\t' && (curbuf->b_p_et || p_sta))
    {
	stuffnumReadbuff(cap->count1);
	stuffcharReadbuff('R');
	stuffcharReadbuff('\t');
	stuffcharReadbuff(ESC);
	return;
    }
    if (u_save_cursor() == FAIL)
	return;
    if (had_ctrl_v != Ctrl_V && (cap->nchar == '\r' || cap->nchar == '\n'))
    {
	(void)del_chars(cap->count1, FALSE);	 
	stuffcharReadbuff('\r');
	stuffcharReadbuff(ESC);
	invoke_edit(cap, TRUE, 'r', FALSE);
    }
    else
    {
	prep_redo(cap->oap->regname, cap->count1,
				       NUL, 'r', NUL, had_ctrl_v, cap->nchar);
	curbuf->b_op_start = curwin->w_cursor;
	if (has_mbyte)
	{
	    int		old_State = State;
	    if (cap->ncharC1 != 0)
		AppendCharToRedobuff(cap->ncharC1);
	    if (cap->ncharC2 != 0)
		AppendCharToRedobuff(cap->ncharC2);
	    for (n = cap->count1; n > 0; --n)
	    {
		State = REPLACE;
		if (cap->nchar == Ctrl_E || cap->nchar == Ctrl_Y)
		{
		    int c = ins_copychar(curwin->w_cursor.lnum
					   + (cap->nchar == Ctrl_Y ? -1 : 1));
		    if (c != NUL)
			ins_char(c);
		    else
			++curwin->w_cursor.col;
		}
		else
		    ins_char(cap->nchar);
		State = old_State;
		if (cap->ncharC1 != 0)
		    ins_char(cap->ncharC1);
		if (cap->ncharC2 != 0)
		    ins_char(cap->ncharC2);
	    }
	}
	else
	{
	    for (n = cap->count1; n > 0; --n)
	    {
		ptr = ml_get_buf(curbuf, curwin->w_cursor.lnum, TRUE);
		if (cap->nchar == Ctrl_E || cap->nchar == Ctrl_Y)
		{
		  int c = ins_copychar(curwin->w_cursor.lnum
					   + (cap->nchar == Ctrl_Y ? -1 : 1));
		  if (c != NUL)
		    ptr[curwin->w_cursor.col] = c;
		}
		else
		    ptr[curwin->w_cursor.col] = cap->nchar;
		if (p_sm && msg_silent == 0)
		    showmatch(cap->nchar);
		++curwin->w_cursor.col;
	    }
#ifdef FEAT_NETBEANS_INTG
	    if (netbeans_active())
	    {
		colnr_T  start = (colnr_T)(curwin->w_cursor.col - cap->count1);
		netbeans_removed(curbuf, curwin->w_cursor.lnum, start,
							   (long)cap->count1);
		netbeans_inserted(curbuf, curwin->w_cursor.lnum, start,
					       &ptr[start], (int)cap->count1);
	    }
#endif
	    changed_bytes(curwin->w_cursor.lnum,
			       (colnr_T)(curwin->w_cursor.col - cap->count1));
	}
	--curwin->w_cursor.col;	     
	if (has_mbyte)
	    mb_adjust_cursor();
	curbuf->b_op_end = curwin->w_cursor;
	curwin->w_set_curswant = TRUE;
	set_last_insert(cap->nchar);
    }
}