diff_redraw(
    int		dofold)	     
{
    win_T	*wp;
    win_T	*wp_other = NULL;
    int		used_max_fill_other = FALSE;
    int		used_max_fill_curwin = FALSE;
    int		n;
    need_diff_redraw = FALSE;
    FOR_ALL_WINDOWS(wp)
	if (wp->w_p_diff)
	{
	    redraw_win_later(wp, SOME_VALID);
	    if (wp != curwin)
		wp_other = wp;
#ifdef FEAT_FOLDING
	    if (dofold && foldmethodIsDiff(wp))
		foldUpdateAll(wp);
#endif
	    n = diff_check(wp, wp->w_topline);
	    if ((wp != curwin && wp->w_topfill > 0) || n > 0)
	    {
		if (wp->w_topfill > n)
		    wp->w_topfill = (n < 0 ? 0 : n);
		else if (n > 0 && n > wp->w_topfill)
		{
		    wp->w_topfill = n;
		    if (wp == curwin)
			used_max_fill_curwin = TRUE;
		    else if (wp_other != NULL)
			used_max_fill_other = TRUE;
		}
		check_topfill(wp, FALSE);
	    }
	}
    if (wp_other != NULL && curwin->w_p_scb)
    {
	if (used_max_fill_curwin)
	    diff_set_topline(wp_other, curwin);
	else if (used_max_fill_other)
	    diff_set_topline(curwin, wp_other);
    }
}