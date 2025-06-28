op_replace(oparg_T *oap, int c)
{
    int			n, numc;
    int			num_chars;
    char_u		*newp, *oldp;
    size_t		oldlen;
    struct block_def	bd;
    char_u		*after_p = NULL;
    int			had_ctrl_v_cr = FALSE;
    if ((curbuf->b_ml.ml_flags & ML_EMPTY ) || oap->empty)
	return OK;	     
    if (c == REPLACE_CR_NCHAR)
    {
	had_ctrl_v_cr = TRUE;
	c = CAR;
    }
    else if (c == REPLACE_NL_NCHAR)
    {
	had_ctrl_v_cr = TRUE;
	c = NL;
    }
    if (has_mbyte)
	mb_adjust_opend(oap);
    if (u_save((linenr_T)(oap->start.lnum - 1),
				       (linenr_T)(oap->end.lnum + 1)) == FAIL)
	return FAIL;
    if (oap->block_mode)
    {
	bd.is_MAX = (curwin->w_curswant == MAXCOL);
	for ( ; curwin->w_cursor.lnum <= oap->end.lnum; ++curwin->w_cursor.lnum)
	{
	    curwin->w_cursor.col = 0;   
	    block_prep(oap, &bd, curwin->w_cursor.lnum, TRUE);
	    if (bd.textlen == 0 && (!virtual_op || bd.is_MAX))
		continue;	     
	    if (virtual_op && bd.is_short && *bd.textstart == NUL)
	    {
		pos_T vpos;
		vpos.lnum = curwin->w_cursor.lnum;
		getvpos(&vpos, oap->start_vcol);
		bd.startspaces += vpos.coladd;
		n = bd.startspaces;
	    }
	    else
		n = (bd.startspaces ? bd.start_char_vcols - 1 : 0);
	    n += (bd.endspaces
		    && !bd.is_oneChar
		    && bd.end_char_vcols > 0) ? bd.end_char_vcols - 1 : 0;
	    numc = oap->end_vcol - oap->start_vcol + 1;
	    if (bd.is_short && (!virtual_op || bd.is_MAX))
		numc -= (oap->end_vcol - bd.end_vcol) + 1;
	    if ((*mb_char2cells)(c) > 1)
	    {
		if ((numc & 1) && !bd.is_short)
		{
		    ++bd.endspaces;
		    ++n;
		}
		numc = numc / 2;
	    }
	    num_chars = numc;
	    numc *= (*mb_char2len)(c);
	    n += numc - bd.textlen;
	    oldp = ml_get_curline();
	    oldlen = STRLEN(oldp);
	    newp = alloc(oldlen + 1 + n);
	    if (newp == NULL)
		continue;
	    vim_memset(newp, NUL, (size_t)(oldlen + 1 + n));
	    mch_memmove(newp, oldp, (size_t)bd.textcol);
	    oldp += bd.textcol + bd.textlen;
	    vim_memset(newp + bd.textcol, ' ', (size_t)bd.startspaces);
	    if (had_ctrl_v_cr || (c != '\r' && c != '\n'))
	    {
		if (has_mbyte)
		{
		    n = (int)STRLEN(newp);
		    while (--num_chars >= 0)
			n += (*mb_char2bytes)(c, newp + n);
		}
		else
		    vim_memset(newp + STRLEN(newp), c, (size_t)numc);
		if (!bd.is_short)
		{
		    vim_memset(newp + STRLEN(newp), ' ', (size_t)bd.endspaces);
		    STRMOVE(newp + STRLEN(newp), oldp);
		}
	    }
	    else
	    {
		after_p = alloc(oldlen + 1 + n - STRLEN(newp));
		if (after_p != NULL)
		    STRMOVE(after_p, oldp);
	    }
	    ml_replace(curwin->w_cursor.lnum, newp, FALSE);
	    if (after_p != NULL)
	    {
		ml_append(curwin->w_cursor.lnum++, after_p, 0, FALSE);
		appended_lines_mark(curwin->w_cursor.lnum, 1L);
		oap->end.lnum++;
		vim_free(after_p);
	    }
	}
    }
    else
    {
	if (oap->motion_type == MLINE)
	{
	    oap->start.col = 0;
	    curwin->w_cursor.col = 0;
	    oap->end.col = (colnr_T)STRLEN(ml_get(oap->end.lnum));
	    if (oap->end.col)
		--oap->end.col;
	}
	else if (!oap->inclusive)
	    dec(&(oap->end));
	while (LTOREQ_POS(curwin->w_cursor, oap->end))
	{
	    n = gchar_cursor();
	    if (n != NUL)
	    {
		int new_byte_len = (*mb_char2len)(c);
		int old_byte_len = mb_ptr2len(ml_get_cursor());
		if (new_byte_len > 1 || old_byte_len > 1)
		{
		    if (curwin->w_cursor.lnum == oap->end.lnum)
			oap->end.col += new_byte_len - old_byte_len;
		    replace_character(c);
		}
		else
		{
		    if (n == TAB)
		    {
			int end_vcol = 0;
			if (curwin->w_cursor.lnum == oap->end.lnum)
			{
			    end_vcol = getviscol2(oap->end.col,
							     oap->end.coladd);
			}
			coladvance_force(getviscol());
			if (curwin->w_cursor.lnum == oap->end.lnum)
			    getvpos(&oap->end, end_vcol);
		    }
		    PBYTE(curwin->w_cursor, c);
		}
	    }
	    else if (virtual_op && curwin->w_cursor.lnum == oap->end.lnum)
	    {
		int virtcols = oap->end.coladd;
		if (curwin->w_cursor.lnum == oap->start.lnum
			&& oap->start.col == oap->end.col && oap->start.coladd)
		    virtcols -= oap->start.coladd;
		coladvance_force(getviscol2(oap->end.col, oap->end.coladd) + 1);
		curwin->w_cursor.col -= (virtcols + 1);
		for (; virtcols >= 0; virtcols--)
		{
		    if ((*mb_char2len)(c) > 1)
		       replace_character(c);
		    else
			PBYTE(curwin->w_cursor, c);
		   if (inc(&curwin->w_cursor) == -1)
		       break;
		}
	    }
	    if (inc_cursor() == -1)
		break;
	}
    }
    curwin->w_cursor = oap->start;
    check_cursor();
    changed_lines(oap->start.lnum, oap->start.col, oap->end.lnum + 1, 0L);
    if ((cmdmod.cmod_flags & CMOD_LOCKMARKS) == 0)
    {
	curbuf->b_op_start = oap->start;
	curbuf->b_op_end = oap->end;
    }
    return OK;
}