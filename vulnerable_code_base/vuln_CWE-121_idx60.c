spell_dump_compl(
    char_u	*pat,	     
    int		ic,	     
    int		*dir,	     
    int		dumpflags_arg)	 
{
    langp_T	*lp;
    slang_T	*slang;
    idx_T	arridx[MAXWLEN];
    int		curi[MAXWLEN];
    char_u	word[MAXWLEN];
    int		c;
    char_u	*byts;
    idx_T	*idxs;
    linenr_T	lnum = 0;
    int		round;
    int		depth;
    int		n;
    int		flags;
    char_u	*region_names = NULL;	     
    int		do_region = TRUE;	     
    char_u	*p;
    int		lpi;
    int		dumpflags = dumpflags_arg;
    int		patlen;
    if (pat != NULL)
    {
	if (ic)
	    dumpflags |= DUMPFLAG_ICASE;
	else
	{
	    n = captype(pat, NULL);
	    if (n == WF_ONECAP)
		dumpflags |= DUMPFLAG_ONECAP;
	    else if (n == WF_ALLCAP && (int)STRLEN(pat) > mb_ptr2len(pat))
		dumpflags |= DUMPFLAG_ALLCAP;
	}
    }
    for (lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi)
    {
	lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
	p = lp->lp_slang->sl_regions;
	if (p[0] != 0)
	{
	    if (region_names == NULL)	     
		region_names = p;
	    else if (STRCMP(region_names, p) != 0)
	    {
		do_region = FALSE;	     
		break;
	    }
	}
    }
    if (do_region && region_names != NULL)
    {
	if (pat == NULL)
	{
	    vim_snprintf((char *)IObuff, IOSIZE, "/regions=%s", region_names);
	    ml_append(lnum++, IObuff, (colnr_T)0, FALSE);
	}
    }
    else
	do_region = FALSE;
    for (lpi = 0; lpi < curwin->w_s->b_langp.ga_len; ++lpi)
    {
	lp = LANGP_ENTRY(curwin->w_s->b_langp, lpi);
	slang = lp->lp_slang;
	if (slang->sl_fbyts == NULL)	     
	    continue;
	if (pat == NULL)
	{
	    vim_snprintf((char *)IObuff, IOSIZE, "# file: %s", slang->sl_fname);
	    ml_append(lnum++, IObuff, (colnr_T)0, FALSE);
	}
	if (pat != NULL && slang->sl_pbyts == NULL)
	    patlen = (int)STRLEN(pat);
	else
	    patlen = -1;
	for (round = 1; round <= 2; ++round)
	{
	    if (round == 1)
	    {
		dumpflags &= ~DUMPFLAG_KEEPCASE;
		byts = slang->sl_fbyts;
		idxs = slang->sl_fidxs;
	    }
	    else
	    {
		dumpflags |= DUMPFLAG_KEEPCASE;
		byts = slang->sl_kbyts;
		idxs = slang->sl_kidxs;
	    }
	    if (byts == NULL)
		continue;		 
	    depth = 0;
	    arridx[0] = 0;
	    curi[0] = 1;
	    while (depth >= 0 && !got_int
				  && (pat == NULL || !ins_compl_interrupted()))
	    {
		if (curi[depth] > byts[arridx[depth]])
		{
		    --depth;
		    line_breakcheck();
		    ins_compl_check_keys(50, FALSE);
		}
		else
		{
		    n = arridx[depth] + curi[depth];
		    ++curi[depth];
		    c = byts[n];
		    if (c == 0)
		    {
			flags = (int)idxs[n];
			if ((round == 2 || (flags & WF_KEEPCAP) == 0)
				&& (flags & WF_NEEDCOMP) == 0
				&& (do_region
				    || (flags & WF_REGION) == 0
				    || (((unsigned)flags >> 16)
						       & lp->lp_region) != 0))
			{
			    word[depth] = NUL;
			    if (!do_region)
				flags &= ~WF_REGION;
			    c = (unsigned)flags >> 24;
			    if (c == 0 || curi[depth] == 2)
			    {
				dump_word(slang, word, pat, dir,
						      dumpflags, flags, lnum);
				if (pat == NULL)
				    ++lnum;
			    }
			    if (c != 0)
				lnum = dump_prefixes(slang, word, pat, dir,
						      dumpflags, flags, lnum);
			}
		    }
		    else
		    {
			word[depth++] = c;
			arridx[depth] = idxs[n];
			curi[depth] = 1;
			if (depth <= patlen
					&& MB_STRNICMP(word, pat, depth) != 0)
			    --depth;
		    }
		}
	    }
	}
    }
}