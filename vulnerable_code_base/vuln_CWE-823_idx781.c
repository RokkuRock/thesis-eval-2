vgr_match_buflines(
	qf_list_T   *qfl,
	char_u	    *fname,
	buf_T	    *buf,
	char_u	    *spat,
	regmmatch_T *regmatch,
	long	    *tomatch,
	int	    duplicate_name,
	int	    flags)
{
    int		found_match = FALSE;
    long	lnum;
    colnr_T	col;
    int		pat_len = (int)STRLEN(spat);
    for (lnum = 1; lnum <= buf->b_ml.ml_line_count && *tomatch > 0; ++lnum)
    {
	col = 0;
	if (!(flags & VGR_FUZZY))
	{
	    while (vim_regexec_multi(regmatch, curwin, buf, lnum,
			col, NULL) > 0)
	    {
		if (qf_add_entry(qfl,
			    NULL,	 
			    fname,
			    NULL,
			    duplicate_name ? 0 : buf->b_fnum,
			    ml_get_buf(buf,
				regmatch->startpos[0].lnum + lnum, FALSE),
			    regmatch->startpos[0].lnum + lnum,
			    regmatch->endpos[0].lnum + lnum,
			    regmatch->startpos[0].col + 1,
			    regmatch->endpos[0].col + 1,
			    FALSE,	 
			    NULL,	 
			    0,		 
			    0,		 
			    TRUE	 
			    ) == QF_FAIL)
		{
		    got_int = TRUE;
		    break;
		}
		found_match = TRUE;
		if (--*tomatch == 0)
		    break;
		if ((flags & VGR_GLOBAL) == 0
			|| regmatch->endpos[0].lnum > 0)
		    break;
		col = regmatch->endpos[0].col
		    + (col == regmatch->endpos[0].col);
		if (col > (colnr_T)STRLEN(ml_get_buf(buf, lnum, FALSE)))
		    break;
	    }
	}
	else
	{
	    char_u  *str = ml_get_buf(buf, lnum, FALSE);
	    int	    score;
	    int_u   matches[MAX_FUZZY_MATCHES];
	    int_u   sz = ARRAY_LENGTH(matches);
	    while (fuzzy_match(str + col, spat, FALSE, &score, matches, sz) > 0)
	    {
		if (qf_add_entry(qfl,
			    NULL,	 
			    fname,
			    NULL,
			    duplicate_name ? 0 : buf->b_fnum,
			    str,
			    lnum,
			    0,
			    matches[0] + col + 1,
			    0,
			    FALSE,	 
			    NULL,	 
			    0,		 
			    0,		 
			    TRUE	 
			    ) == QF_FAIL)
		{
		    got_int = TRUE;
		    break;
		}
		found_match = TRUE;
		if (--*tomatch == 0)
		    break;
		if ((flags & VGR_GLOBAL) == 0)
		    break;
		col = matches[pat_len - 1] + col + 1;
		if (col > (colnr_T)STRLEN(str))
		    break;
	    }
	}
	line_breakcheck();
	if (got_int)
	    break;
    }
    return found_match;
}