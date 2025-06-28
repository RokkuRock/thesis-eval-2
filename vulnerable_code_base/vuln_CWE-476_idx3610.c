buflist_findpat(
    char_u	*pattern,
    char_u	*pattern_end,	 
    int		unlisted,	 
    int		diffmode UNUSED,  
    int		curtab_only)	 
{
    buf_T	*buf;
    int		match = -1;
    int		find_listed;
    char_u	*pat;
    char_u	*patend;
    int		attempt;
    char_u	*p;
    int		toggledollar;
    if ((pattern_end == pattern + 1 && (*pattern == '%' || *pattern == '#'))
	    || (in_vim9script() && pattern_end == pattern + 2
				    && pattern[0] == '%' && pattern[1] == '%'))
    {
	if (*pattern == '#' || pattern_end == pattern + 2)
	    match = curwin->w_alt_fnum;
	else
	    match = curbuf->b_fnum;
#ifdef FEAT_DIFF
	if (diffmode && !diff_mode_buf(buflist_findnr(match)))
	    match = -1;
#endif
    }
    else
    {
	pat = file_pat_to_reg_pat(pattern, pattern_end, NULL, FALSE);
	if (pat == NULL)
	    return -1;
	patend = pat + STRLEN(pat) - 1;
	toggledollar = (patend > pat && *patend == '$');
	find_listed = TRUE;
	for (;;)
	{
	    for (attempt = 0; attempt <= 3; ++attempt)
	    {
		regmatch_T	regmatch;
		if (toggledollar)
		    *patend = (attempt < 2) ? NUL : '$';  
		p = pat;
		if (*p == '^' && !(attempt & 1))	  
		    ++p;
		regmatch.regprog = vim_regcomp(p, magic_isset() ? RE_MAGIC : 0);
		if (regmatch.regprog == NULL)
		{
		    vim_free(pat);
		    return -1;
		}
		FOR_ALL_BUFS_FROM_LAST(buf)
		    if (buf->b_p_bl == find_listed
#ifdef FEAT_DIFF
			    && (!diffmode || diff_mode_buf(buf))
#endif
			    && buflist_match(&regmatch, buf, FALSE) != NULL)
		    {
			if (curtab_only)
			{
			    win_T	*wp;
			    FOR_ALL_WINDOWS(wp)
				if (wp->w_buffer == buf)
				    break;
			    if (wp == NULL)
				continue;
			}
			if (match >= 0)		 
			{
			    match = -2;
			    break;
			}
			match = buf->b_fnum;	 
		    }
		vim_regfree(regmatch.regprog);
		if (match >= 0)			 
		    break;
	    }
	    if (!unlisted || !find_listed || match != -1)
		break;
	    find_listed = FALSE;
	}
	vim_free(pat);
    }
    if (match == -2)
	semsg(_(e_more_than_one_match_for_str), pattern);
    else if (match < 0)
	semsg(_(e_no_matching_buffer_for_str), pattern);
    return match;
}