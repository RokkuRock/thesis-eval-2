diff_mark_adjust_tp(
    tabpage_T	*tp,
    int		idx,
    linenr_T	line1,
    linenr_T	line2,
    long	amount,
    long	amount_after)
{
    diff_T	*dp;
    diff_T	*dprev;
    diff_T	*dnext;
    int		i;
    int		inserted, deleted;
    int		n, off;
    linenr_T	last;
    linenr_T	lnum_deleted = line1;	 
    int		check_unchanged;
    if (diff_internal())
    {
	tp->tp_diff_invalid = TRUE;
	tp->tp_diff_update = TRUE;
    }
    if (line2 == MAXLNUM)
    {
	inserted = amount;
	deleted = 0;
    }
    else if (amount_after > 0)
    {
	inserted = amount_after;
	deleted = 0;
    }
    else
    {
	inserted = 0;
	deleted = -amount_after;
    }
    dprev = NULL;
    dp = tp->tp_first_diff;
    for (;;)
    {
	if ((dp == NULL || dp->df_lnum[idx] - 1 > line2
		    || (line2 == MAXLNUM && dp->df_lnum[idx] > line1))
		&& (dprev == NULL
		    || dprev->df_lnum[idx] + dprev->df_count[idx] < line1)
		&& !diff_busy)
	{
	    dnext = diff_alloc_new(tp, dprev, dp);
	    if (dnext == NULL)
		return;
	    dnext->df_lnum[idx] = line1;
	    dnext->df_count[idx] = inserted;
	    for (i = 0; i < DB_COUNT; ++i)
		if (tp->tp_diffbuf[i] != NULL && i != idx)
		{
		    if (dprev == NULL)
			dnext->df_lnum[i] = line1;
		    else
			dnext->df_lnum[i] = line1
			    + (dprev->df_lnum[i] + dprev->df_count[i])
			    - (dprev->df_lnum[idx] + dprev->df_count[idx]);
		    dnext->df_count[i] = deleted;
		}
	}
	if (dp == NULL)
	    break;
	last = dp->df_lnum[idx] + dp->df_count[idx] - 1;
	if (last >= line1 - 1)
	{
	    if (dp->df_lnum[idx] - (deleted + inserted != 0) > line2)
	    {
		if (amount_after == 0)
		    break;	 
		dp->df_lnum[idx] += amount_after;
	    }
	    else
	    {
		check_unchanged = FALSE;
		if (deleted > 0)
		{
		    if (dp->df_lnum[idx] >= line1)
		    {
			off = dp->df_lnum[idx] - lnum_deleted;
			if (last <= line2)
			{
			    if (dp->df_next != NULL
				    && dp->df_next->df_lnum[idx] - 1 <= line2)
			    {
				n = dp->df_next->df_lnum[idx] - lnum_deleted;
				deleted -= n;
				n -= dp->df_count[idx];
				lnum_deleted = dp->df_next->df_lnum[idx];
			    }
			    else
				n = deleted - dp->df_count[idx];
			    dp->df_count[idx] = 0;
			}
			else
			{
			    n = off;
			    dp->df_count[idx] -= line2 - dp->df_lnum[idx] + 1;
			    check_unchanged = TRUE;
			}
			dp->df_lnum[idx] = line1;
		    }
		    else
		    {
			off = 0;
			if (last < line2)
			{
			    dp->df_count[idx] -= last - lnum_deleted + 1;
			    if (dp->df_next != NULL
				    && dp->df_next->df_lnum[idx] - 1 <= line2)
			    {
				n = dp->df_next->df_lnum[idx] - 1 - last;
				deleted -= dp->df_next->df_lnum[idx]
							       - lnum_deleted;
				lnum_deleted = dp->df_next->df_lnum[idx];
			    }
			    else
				n = line2 - last;
			    check_unchanged = TRUE;
			}
			else
			{
			    n = 0;
			    dp->df_count[idx] -= deleted;
			}
		    }
		    for (i = 0; i < DB_COUNT; ++i)
			if (tp->tp_diffbuf[i] != NULL && i != idx)
			{
			    dp->df_lnum[i] -= off;
			    dp->df_count[i] += n;
			}
		}
		else
		{
		    if (dp->df_lnum[idx] <= line1)
		    {
			dp->df_count[idx] += inserted;
			check_unchanged = TRUE;
		    }
		    else
			dp->df_lnum[idx] += inserted;
		}
		if (check_unchanged)
		    diff_check_unchanged(tp, dp);
	    }
	}
	if (dprev != NULL && dprev->df_lnum[idx] + dprev->df_count[idx]
							  == dp->df_lnum[idx])
	{
	    for (i = 0; i < DB_COUNT; ++i)
		if (tp->tp_diffbuf[i] != NULL)
		    dprev->df_count[i] += dp->df_count[i];
	    dprev->df_next = dp->df_next;
	    vim_free(dp);
	    dp = dprev->df_next;
	}
	else
	{
	    dprev = dp;
	    dp = dp->df_next;
	}
    }
    dprev = NULL;
    dp = tp->tp_first_diff;
    while (dp != NULL)
    {
	for (i = 0; i < DB_COUNT; ++i)
	    if (tp->tp_diffbuf[i] != NULL && dp->df_count[i] != 0)
		break;
	if (i == DB_COUNT)
	{
	    dnext = dp->df_next;
	    vim_free(dp);
	    dp = dnext;
	    if (dprev == NULL)
		tp->tp_first_diff = dnext;
	    else
		dprev->df_next = dnext;
	}
	else
	{
	    dprev = dp;
	    dp = dp->df_next;
	}
    }
    if (tp == curtab)
    {
	need_diff_redraw = TRUE;
	diff_need_scrollbind = TRUE;
    }
}