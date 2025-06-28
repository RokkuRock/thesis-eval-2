find_ucmd(
    exarg_T	*eap,
    char_u	*p,	  
    int		*full,	  
    expand_T	*xp,	  
    int		*complp)  
{
    int		len = (int)(p - eap->cmd);
    int		j, k, matchlen = 0;
    ucmd_T	*uc;
    int		found = FALSE;
    int		possible = FALSE;
    char_u	*cp, *np;	     
    garray_T	*gap;
    int		amb_local = FALSE;   
    gap =
#ifdef FEAT_CMDWIN
	is_in_cmdwin() ? &prevwin->w_buffer->b_ucmds :
#endif
	&curbuf->b_ucmds;
    for (;;)
    {
	for (j = 0; j < gap->ga_len; ++j)
	{
	    uc = USER_CMD_GA(gap, j);
	    cp = eap->cmd;
	    np = uc->uc_name;
	    k = 0;
	    while (k < len && *np != NUL && *cp++ == *np++)
		k++;
	    if (k == len || (*np == NUL && vim_isdigit(eap->cmd[k])))
	    {
		if (k == len && found && *np != NUL)
		{
		    if (gap == &ucmds)
			return NULL;
		    amb_local = TRUE;
		}
		if (!found || (k == len && *np == NUL))
		{
		    if (k == len)
			found = TRUE;
		    else
			possible = TRUE;
		    if (gap == &ucmds)
			eap->cmdidx = CMD_USER;
		    else
			eap->cmdidx = CMD_USER_BUF;
		    eap->argt = (long)uc->uc_argt;
		    eap->useridx = j;
		    eap->addr_type = uc->uc_addr_type;
		    if (complp != NULL)
			*complp = uc->uc_compl;
# ifdef FEAT_EVAL
		    if (xp != NULL)
		    {
			xp->xp_arg = uc->uc_compl_arg;
			xp->xp_script_ctx = uc->uc_script_ctx;
			xp->xp_script_ctx.sc_lnum += SOURCING_LNUM;
		    }
# endif
		    matchlen = k;
		    if (k == len && *np == NUL)
		    {
			if (full != NULL)
			    *full = TRUE;
			amb_local = FALSE;
			break;
		    }
		}
	    }
	}
	if (j < gap->ga_len || gap == &ucmds)
	    break;
	gap = &ucmds;
    }
    if (amb_local)
    {
	if (xp != NULL)
	    xp->xp_context = EXPAND_UNSUCCESSFUL;
	return NULL;
    }
    if (found || possible)
	return p + (matchlen - len);
    return p;
}