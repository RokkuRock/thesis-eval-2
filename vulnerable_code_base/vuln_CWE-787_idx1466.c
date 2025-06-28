op_insert(oparg_T *oap, long count1)
{
    long		ins_len, pre_textlen = 0;
    char_u		*firstline, *ins_text;
    colnr_T		ind_pre_col = 0, ind_post_col;
    int			ind_pre_vcol = 0, ind_post_vcol = 0;
    struct block_def	bd;
    int			i;
    pos_T		t1;
    pos_T		start_insert;
    int			offset = 0;
    bd.is_MAX = (curwin->w_curswant == MAXCOL);
    curwin->w_cursor.lnum = oap->start.lnum;
    update_screen(INVERTED);
    if (oap->block_mode)
    {
	if (curwin->w_cursor.coladd > 0)
	{
	    int		old_ve_flags = curwin->w_ve_flags;
	    if (u_save_cursor() == FAIL)
		return;
	    curwin->w_ve_flags = VE_ALL;
	    coladvance_force(oap->op_type == OP_APPEND
					   ? oap->end_vcol + 1 : getviscol());
	    if (oap->op_type == OP_APPEND)
		--curwin->w_cursor.col;
	    curwin->w_ve_flags = old_ve_flags;
	}
	block_prep(oap, &bd, oap->start.lnum, TRUE);
	ind_pre_col = (colnr_T)getwhitecols_curline();
	ind_pre_vcol = get_indent();
	firstline = ml_get(oap->start.lnum) + bd.textcol;
	if (oap->op_type == OP_APPEND)
	    firstline += bd.textlen;
	pre_textlen = (long)STRLEN(firstline);
    }
    if (oap->op_type == OP_APPEND)
    {
	if (oap->block_mode && curwin->w_cursor.coladd == 0)
	{
	    curwin->w_set_curswant = TRUE;
	    while (*ml_get_cursor() != NUL
		    && (curwin->w_cursor.col < bd.textcol + bd.textlen))
		++curwin->w_cursor.col;
	    if (bd.is_short && !bd.is_MAX)
	    {
		if (u_save_cursor() == FAIL)
		    return;
		for (i = 0; i < bd.endspaces; ++i)
		    ins_char(' ');
		bd.textlen += bd.endspaces;
	    }
	}
	else
	{
	    curwin->w_cursor = oap->end;
	    check_cursor_col();
	    if (!LINEEMPTY(curwin->w_cursor.lnum)
		    && oap->start_vcol != oap->end_vcol)
		inc_cursor();
	}
    }
    t1 = oap->start;
    start_insert = curwin->w_cursor;
    (void)edit(NUL, FALSE, (linenr_T)count1);
    if (t1.lnum == curbuf->b_op_start_orig.lnum
	    && LT_POS(curbuf->b_op_start_orig, t1))
	oap->start = curbuf->b_op_start_orig;
    if (curwin->w_cursor.lnum != oap->start.lnum || got_int)
	return;
    if (oap->block_mode)
    {
	struct block_def	bd2;
	int			did_indent = FALSE;
	size_t			len;
	int			add;
	ind_post_col = (colnr_T)getwhitecols_curline();
	if (curbuf->b_op_start.col > ind_pre_col && ind_post_col > ind_pre_col)
	{
	    bd.textcol += ind_post_col - ind_pre_col;
	    ind_post_vcol = get_indent();
	    bd.start_vcol += ind_post_vcol - ind_pre_vcol;
	    did_indent = TRUE;
	}
	if (oap->start.lnum == curbuf->b_op_start_orig.lnum
						  && !bd.is_MAX && !did_indent)
	{
	    int t = getviscol2(curbuf->b_op_start_orig.col,
					       curbuf->b_op_start_orig.coladd);
	    if (!bd.is_MAX)
	    {
		if (oap->op_type == OP_INSERT
			&& oap->start.col + oap->start.coladd
				!= curbuf->b_op_start_orig.col
					      + curbuf->b_op_start_orig.coladd)
		{
		    oap->start.col = curbuf->b_op_start_orig.col;
		    pre_textlen -= t - oap->start_vcol;
		    oap->start_vcol = t;
		}
		else if (oap->op_type == OP_APPEND
			&& oap->end.col + oap->end.coladd
				>= curbuf->b_op_start_orig.col
					      + curbuf->b_op_start_orig.coladd)
		{
		    oap->start.col = curbuf->b_op_start_orig.col;
		    pre_textlen += bd.textlen;
		    pre_textlen -= t - oap->start_vcol;
		    oap->start_vcol = t;
		    oap->op_type = OP_INSERT;
		}
	    }
	    else if (bd.is_MAX && oap->op_type == OP_APPEND)
	    {
		pre_textlen += bd.textlen;
		pre_textlen -= t - oap->start_vcol;
	    }
	}
	if (did_indent && bd.textcol - ind_post_col > 0)
	{
	    oap->start.col += ind_post_col - ind_pre_col;
	    oap->start_vcol += ind_post_vcol - ind_pre_vcol;
	    oap->end.col += ind_post_col - ind_pre_col;
	    oap->end_vcol += ind_post_vcol - ind_pre_vcol;
	}
	block_prep(oap, &bd2, oap->start.lnum, TRUE);
	if (did_indent && bd.textcol - ind_post_col > 0)
	{
	    oap->start.col -= ind_post_col - ind_pre_col;
	    oap->start_vcol -= ind_post_vcol - ind_pre_vcol;
	    oap->end.col -= ind_post_col - ind_pre_col;
	    oap->end_vcol -= ind_post_vcol - ind_pre_vcol;
	}
	if (!bd.is_MAX || bd2.textlen < bd.textlen)
	{
	    if (oap->op_type == OP_APPEND)
	    {
		pre_textlen += bd2.textlen - bd.textlen;
		if (bd2.endspaces)
		    --bd2.textlen;
	    }
	    bd.textcol = bd2.textcol;
	    bd.textlen = bd2.textlen;
	}
	firstline = ml_get(oap->start.lnum);
	len = STRLEN(firstline);
	add = bd.textcol;
	if (oap->op_type == OP_APPEND)
	{
	    add += bd.textlen;
	    if (bd.is_MAX
		&& (start_insert.lnum == Insstart.lnum
					   && start_insert.col > Insstart.col))
	    {
		offset = (start_insert.col - Insstart.col);
		add -= offset;
		if (oap->end_vcol > offset)
		    oap->end_vcol -= (offset + 1);
		else
		    return;
	    }
	}
	if ((size_t)add > len)
	    firstline += len;   
	else
	    firstline += add;
	if (pre_textlen >= 0 && (ins_len =
			   (long)STRLEN(firstline) - pre_textlen - offset) > 0)
	{
	    ins_text = vim_strnsave(firstline, ins_len);
	    if (ins_text != NULL)
	    {
		if (u_save(oap->start.lnum,
					 (linenr_T)(oap->end.lnum + 1)) == OK)
		    block_insert(oap, ins_text, (oap->op_type == OP_INSERT),
									 &bd);
		curwin->w_cursor.col = oap->start.col;
		check_cursor();
		vim_free(ins_text);
	    }
	}
    }
}