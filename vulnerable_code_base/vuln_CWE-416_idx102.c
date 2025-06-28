ex_diffgetput(exarg_T *eap)
{
    linenr_T	lnum;
    int		count;
    linenr_T	off = 0;
    diff_T	*dp;
    diff_T	*dprev;
    diff_T	*dfree;
    int		idx_cur;
    int		idx_other;
    int		idx_from;
    int		idx_to;
    int		i;
    int		added;
    char_u	*p;
    aco_save_T	aco;
    buf_T	*buf;
    int		start_skip, end_skip;
    int		new_count;
    int		buf_empty;
    int		found_not_ma = FALSE;
    idx_cur = diff_buf_idx(curbuf);
    if (idx_cur == DB_COUNT)
    {
	emsg(_(e_current_buffer_is_not_in_diff_mode));
	return;
    }
    if (*eap->arg == NUL)
    {
	for (idx_other = 0; idx_other < DB_COUNT; ++idx_other)
	    if (curtab->tp_diffbuf[idx_other] != curbuf
		    && curtab->tp_diffbuf[idx_other] != NULL)
	    {
		if (eap->cmdidx != CMD_diffput
				     || curtab->tp_diffbuf[idx_other]->b_p_ma)
		    break;
		found_not_ma = TRUE;
	    }
	if (idx_other == DB_COUNT)
	{
	    if (found_not_ma)
		emsg(_(e_no_other_buffer_in_diff_mode_is_modifiable));
	    else
		emsg(_(e_no_other_buffer_in_diff_mode));
	    return;
	}
	for (i = idx_other + 1; i < DB_COUNT; ++i)
	    if (curtab->tp_diffbuf[i] != curbuf
		    && curtab->tp_diffbuf[i] != NULL
		    && (eap->cmdidx != CMD_diffput || curtab->tp_diffbuf[i]->b_p_ma))
	    {
		emsg(_(e_more_than_two_buffers_in_diff_mode_dont_know_which_one_to_use));
		return;
	    }
    }
    else
    {
	p = eap->arg + STRLEN(eap->arg);
	while (p > eap->arg && VIM_ISWHITE(p[-1]))
	    --p;
	for (i = 0; vim_isdigit(eap->arg[i]) && eap->arg + i < p; ++i)
	    ;
	if (eap->arg + i == p)	     
	    i = atol((char *)eap->arg);
	else
	{
	    i = buflist_findpat(eap->arg, p, FALSE, TRUE, FALSE);
	    if (i < 0)
		return;		 
	}
	buf = buflist_findnr(i);
	if (buf == NULL)
	{
	    semsg(_(e_cant_find_buffer_str), eap->arg);
	    return;
	}
	if (buf == curbuf)
	    return;		 
	idx_other = diff_buf_idx(buf);
	if (idx_other == DB_COUNT)
	{
	    semsg(_(e_buffer_str_is_not_in_diff_mode), eap->arg);
	    return;
	}
    }
    diff_busy = TRUE;
    if (eap->addr_count == 0)
    {
	if (eap->cmdidx == CMD_diffget
		&& eap->line1 == curbuf->b_ml.ml_line_count
		&& diff_check(curwin, eap->line1) == 0
		&& (eap->line1 == 1 || diff_check(curwin, eap->line1 - 1) == 0))
	    ++eap->line2;
	else if (eap->line1 > 0)
	    --eap->line1;
    }
    if (eap->cmdidx == CMD_diffget)
    {
	idx_from = idx_other;
	idx_to = idx_cur;
    }
    else
    {
	idx_from = idx_cur;
	idx_to = idx_other;
	aucmd_prepbuf(&aco, curtab->tp_diffbuf[idx_other]);
    }
    if (!curbuf->b_changed)
    {
	change_warning(0);
	if (diff_buf_idx(curbuf) != idx_to)
	{
	    emsg(_(e_buffer_changed_unexpectedly));
	    goto theend;
	}
    }
    dprev = NULL;
    for (dp = curtab->tp_first_diff; dp != NULL; )
    {
	if (dp->df_lnum[idx_cur] > eap->line2 + off)
	    break;	 
	dfree = NULL;
	lnum = dp->df_lnum[idx_to];
	count = dp->df_count[idx_to];
	if (dp->df_lnum[idx_cur] + dp->df_count[idx_cur] > eap->line1 + off
		&& u_save(lnum - 1, lnum + count) != FAIL)
	{
	    start_skip = 0;
	    end_skip = 0;
	    if (eap->addr_count > 0)
	    {
		start_skip = eap->line1 + off - dp->df_lnum[idx_cur];
		if (start_skip > 0)
		{
		    if (start_skip > count)
		    {
			lnum += count;
			count = 0;
		    }
		    else
		    {
			count -= start_skip;
			lnum += start_skip;
		    }
		}
		else
		    start_skip = 0;
		end_skip = dp->df_lnum[idx_cur] + dp->df_count[idx_cur] - 1
							 - (eap->line2 + off);
		if (end_skip > 0)
		{
		    if (idx_cur == idx_from)	 
		    {
			i = dp->df_count[idx_cur] - start_skip - end_skip;
			if (count > i)
			    count = i;
		    }
		    else			 
		    {
			count -= end_skip;
			end_skip = dp->df_count[idx_from] - start_skip - count;
			if (end_skip < 0)
			    end_skip = 0;
		    }
		}
		else
		    end_skip = 0;
	    }
	    buf_empty = BUFEMPTY();
	    added = 0;
	    for (i = 0; i < count; ++i)
	    {
		buf_empty = curbuf->b_ml.ml_line_count == 1;
		ml_delete(lnum);
		--added;
	    }
	    for (i = 0; i < dp->df_count[idx_from] - start_skip - end_skip; ++i)
	    {
		linenr_T nr;
		nr = dp->df_lnum[idx_from] + start_skip + i;
		if (nr > curtab->tp_diffbuf[idx_from]->b_ml.ml_line_count)
		    break;
		p = vim_strsave(ml_get_buf(curtab->tp_diffbuf[idx_from],
								  nr, FALSE));
		if (p != NULL)
		{
		    ml_append(lnum + i - 1, p, 0, FALSE);
		    vim_free(p);
		    ++added;
		    if (buf_empty && curbuf->b_ml.ml_line_count == 2)
		    {
			buf_empty = FALSE;
			ml_delete((linenr_T)2);
		    }
		}
	    }
	    new_count = dp->df_count[idx_to] + added;
	    dp->df_count[idx_to] = new_count;
	    if (start_skip == 0 && end_skip == 0)
	    {
		for (i = 0; i < DB_COUNT; ++i)
		    if (curtab->tp_diffbuf[i] != NULL && i != idx_from
								&& i != idx_to
			    && !diff_equal_entry(dp, idx_from, i))
			break;
		if (i == DB_COUNT)
		{
		    dfree = dp;
		    dp = dp->df_next;
		    if (dprev == NULL)
			curtab->tp_first_diff = dp;
		    else
			dprev->df_next = dp;
		}
	    }
	    if (added != 0)
	    {
		mark_adjust(lnum, lnum + count - 1, (long)MAXLNUM, (long)added);
		if (curwin->w_cursor.lnum >= lnum)
		{
		    if (curwin->w_cursor.lnum >= lnum + count)
			curwin->w_cursor.lnum += added;
		    else if (added < 0)
			curwin->w_cursor.lnum = lnum;
		}
	    }
	    changed_lines(lnum, 0, lnum + count, (long)added);
	    if (dfree != NULL)
	    {
#ifdef FEAT_FOLDING
		diff_fold_update(dfree, idx_to);
#endif
		vim_free(dfree);
	    }
	    else
		dp->df_count[idx_to] = new_count;
	    if (idx_cur == idx_to)
		off += added;
	}
	if (dfree == NULL)
	{
	    dprev = dp;
	    dp = dp->df_next;
	}
    }
    if (eap->cmdidx != CMD_diffget)
    {
	if (KeyTyped)
	    u_sync(FALSE);
	aucmd_restbuf(&aco);
    }
theend:
    diff_busy = FALSE;
    if (diff_need_update)
	ex_diffupdate(NULL);
    check_cursor();
    changed_line_abv_curs();
    if (diff_need_update)
	diff_need_update = FALSE;
    else
    {
	diff_redraw(FALSE);
	apply_autocmds(EVENT_DIFFUPDATED, NULL, NULL, FALSE, curbuf);
    }
}