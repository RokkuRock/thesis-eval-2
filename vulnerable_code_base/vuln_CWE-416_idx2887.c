did_set_spelllang(win_T *wp)
{
    garray_T	ga;
    char_u	*splp;
    char_u	*region;
    char_u	region_cp[3];
    int		filename;
    int		region_mask;
    slang_T	*slang;
    int		c;
    char_u	lang[MAXWLEN + 1];
    char_u	spf_name[MAXPATHL];
    int		len;
    char_u	*p;
    int		round;
    char_u	*spf;
    char_u	*use_region = NULL;
    int		dont_use_region = FALSE;
    int		nobreak = FALSE;
    int		i, j;
    langp_T	*lp, *lp2;
    static int	recursive = FALSE;
    char	*ret_msg = NULL;
    char_u	*spl_copy;
    bufref_T	bufref;
    set_bufref(&bufref, wp->w_buffer);
    if (recursive)
	return NULL;
    recursive = TRUE;
    ga_init2(&ga, sizeof(langp_T), 2);
    clear_midword(wp);
    spl_copy = vim_strsave(wp->w_s->b_p_spl);
    if (spl_copy == NULL)
	goto theend;
    wp->w_s->b_cjk = 0;
    for (splp = spl_copy; *splp != NUL; )
    {
	copy_option_part(&splp, lang, MAXWLEN, ",");
	region = NULL;
	len = (int)STRLEN(lang);
	if (!valid_spelllang(lang))
	    continue;
	if (STRCMP(lang, "cjk") == 0)
	{
	    wp->w_s->b_cjk = 1;
	    continue;
	}
	if (len > 4 && fnamecmp(lang + len - 4, ".spl") == 0)
	{
	    filename = TRUE;
	    p = vim_strchr(gettail(lang), '_');
	    if (p != NULL && ASCII_ISALPHA(p[1]) && ASCII_ISALPHA(p[2])
						      && !ASCII_ISALPHA(p[3]))
	    {
		vim_strncpy(region_cp, p + 1, 2);
		mch_memmove(p, p + 3, len - (p - lang) - 2);
		region = region_cp;
	    }
	    else
		dont_use_region = TRUE;
	    FOR_ALL_SPELL_LANGS(slang)
		if (fullpathcmp(lang, slang->sl_fname, FALSE, TRUE) == FPC_SAME)
		    break;
	}
	else
	{
	    filename = FALSE;
	    if (len > 3 && lang[len - 3] == '_')
	    {
		region = lang + len - 2;
		len -= 3;
		lang[len] = NUL;
	    }
	    else
		dont_use_region = TRUE;
	    FOR_ALL_SPELL_LANGS(slang)
		if (STRICMP(lang, slang->sl_name) == 0)
		    break;
	}
	if (region != NULL)
	{
	    if (use_region != NULL && STRCMP(region, use_region) != 0)
		dont_use_region = TRUE;
	    use_region = region;
	}
	if (slang == NULL)
	{
	    if (filename)
		(void)spell_load_file(lang, lang, NULL, FALSE);
	    else
	    {
		spell_load_lang(lang);
		if (!bufref_valid(&bufref))
		{
		    ret_msg = N_(e_spellfilemising_autocommand_deleted_buffer);
		    goto theend;
		}
	    }
	}
	FOR_ALL_SPELL_LANGS(slang)
	    if (filename ? fullpathcmp(lang, slang->sl_fname, FALSE, TRUE)
								    == FPC_SAME
			 : STRICMP(lang, slang->sl_name) == 0)
	    {
		region_mask = REGION_ALL;
		if (!filename && region != NULL)
		{
		    c = find_region(slang->sl_regions, region);
		    if (c == REGION_ALL)
		    {
			if (slang->sl_add)
			{
			    if (*slang->sl_regions != NUL)
				region_mask = 0;
			}
			else
			    smsg(_("Warning: region %s not supported"),
								      region);
		    }
		    else
			region_mask = 1 << c;
		}
		if (region_mask != 0)
		{
		    if (ga_grow(&ga, 1) == FAIL)
		    {
			ga_clear(&ga);
			ret_msg = e_out_of_memory;
			goto theend;
		    }
		    LANGP_ENTRY(ga, ga.ga_len)->lp_slang = slang;
		    LANGP_ENTRY(ga, ga.ga_len)->lp_region = region_mask;
		    ++ga.ga_len;
		    use_midword(slang, wp);
		    if (slang->sl_nobreak)
			nobreak = TRUE;
		}
	    }
    }
    spf = curwin->w_s->b_p_spf;
    for (round = 0; round == 0 || *spf != NUL; ++round)
    {
	if (round == 0)
	{
	    if (int_wordlist == NULL)
		continue;
	    int_wordlist_spl(spf_name);
	}
	else
	{
	    copy_option_part(&spf, spf_name, MAXPATHL - 5, ",");
	    STRCAT(spf_name, ".spl");
	    for (c = 0; c < ga.ga_len; ++c)
	    {
		p = LANGP_ENTRY(ga, c)->lp_slang->sl_fname;
		if (p != NULL && fullpathcmp(spf_name, p, FALSE, TRUE)
								== FPC_SAME)
		    break;
	    }
	    if (c < ga.ga_len)
		continue;
	}
	FOR_ALL_SPELL_LANGS(slang)
	    if (fullpathcmp(spf_name, slang->sl_fname, FALSE, TRUE)
								== FPC_SAME)
		break;
	if (slang == NULL)
	{
	    if (round == 0)
		STRCPY(lang, "internal wordlist");
	    else
	    {
		vim_strncpy(lang, gettail(spf_name), MAXWLEN);
		p = vim_strchr(lang, '.');
		if (p != NULL)
		    *p = NUL;	 
	    }
	    slang = spell_load_file(spf_name, lang, NULL, TRUE);
	    if (slang != NULL && nobreak)
		slang->sl_nobreak = TRUE;
	}
	if (slang != NULL && ga_grow(&ga, 1) == OK)
	{
	    region_mask = REGION_ALL;
	    if (use_region != NULL && !dont_use_region)
	    {
		c = find_region(slang->sl_regions, use_region);
		if (c != REGION_ALL)
		    region_mask = 1 << c;
		else if (*slang->sl_regions != NUL)
		    region_mask = 0;
	    }
	    if (region_mask != 0)
	    {
		LANGP_ENTRY(ga, ga.ga_len)->lp_slang = slang;
		LANGP_ENTRY(ga, ga.ga_len)->lp_sallang = NULL;
		LANGP_ENTRY(ga, ga.ga_len)->lp_replang = NULL;
		LANGP_ENTRY(ga, ga.ga_len)->lp_region = region_mask;
		++ga.ga_len;
		use_midword(slang, wp);
	    }
	}
    }
    ga_clear(&wp->w_s->b_langp);
    wp->w_s->b_langp = ga;
    for (i = 0; i < ga.ga_len; ++i)
    {
	lp = LANGP_ENTRY(ga, i);
	if (lp->lp_slang->sl_sal.ga_len > 0)
	    lp->lp_sallang = lp->lp_slang;
	else
	    for (j = 0; j < ga.ga_len; ++j)
	    {
		lp2 = LANGP_ENTRY(ga, j);
		if (lp2->lp_slang->sl_sal.ga_len > 0
			&& STRNCMP(lp->lp_slang->sl_name,
					      lp2->lp_slang->sl_name, 2) == 0)
		{
		    lp->lp_sallang = lp2->lp_slang;
		    break;
		}
	    }
	if (lp->lp_slang->sl_rep.ga_len > 0)
	    lp->lp_replang = lp->lp_slang;
	else
	    for (j = 0; j < ga.ga_len; ++j)
	    {
		lp2 = LANGP_ENTRY(ga, j);
		if (lp2->lp_slang->sl_rep.ga_len > 0
			&& STRNCMP(lp->lp_slang->sl_name,
					      lp2->lp_slang->sl_name, 2) == 0)
		{
		    lp->lp_replang = lp2->lp_slang;
		    break;
		}
	    }
    }
    redraw_win_later(wp, UPD_NOT_VALID);
theend:
    vim_free(spl_copy);
    recursive = FALSE;
    return ret_msg;
}