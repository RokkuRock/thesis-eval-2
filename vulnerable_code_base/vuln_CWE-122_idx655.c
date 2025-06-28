ins_compl_add(
    char_u	*str,
    int		len,
    char_u	*fname,
    char_u	**cptext,	     
    typval_T	*user_data UNUSED,   
    int		cdir,
    int		flags_arg,
    int		adup)		 
{
    compl_T	*match;
    int		dir = (cdir == 0 ? compl_direction : cdir);
    int		flags = flags_arg;
    if (flags & CP_FAST)
	fast_breakcheck();
    else
	ui_breakcheck();
    if (got_int)
	return FAIL;
    if (len < 0)
	len = (int)STRLEN(str);
    if (compl_first_match != NULL && !adup)
    {
	match = compl_first_match;
	do
	{
	    if (!match_at_original_text(match)
		    && STRNCMP(match->cp_str, str, len) == 0
		    && match->cp_str[len] == NUL)
		return NOTDONE;
	    match = match->cp_next;
	} while (match != NULL && !is_first_match(match));
    }
    ins_compl_del_pum();
    match = ALLOC_CLEAR_ONE(compl_T);
    if (match == NULL)
	return FAIL;
    match->cp_number = -1;
    if (flags & CP_ORIGINAL_TEXT)
	match->cp_number = 0;
    if ((match->cp_str = vim_strnsave(str, len)) == NULL)
    {
	vim_free(match);
	return FAIL;
    }
    if (fname != NULL
	    && compl_curr_match != NULL
	    && compl_curr_match->cp_fname != NULL
	    && STRCMP(fname, compl_curr_match->cp_fname) == 0)
	match->cp_fname = compl_curr_match->cp_fname;
    else if (fname != NULL)
    {
	match->cp_fname = vim_strsave(fname);
	flags |= CP_FREE_FNAME;
    }
    else
	match->cp_fname = NULL;
    match->cp_flags = flags;
    if (cptext != NULL)
    {
	int i;
	for (i = 0; i < CPT_COUNT; ++i)
	    if (cptext[i] != NULL && *cptext[i] != NUL)
		match->cp_text[i] = vim_strsave(cptext[i]);
    }
#ifdef FEAT_EVAL
    if (user_data != NULL)
	match->cp_user_data = *user_data;
#endif
    if (compl_first_match == NULL)
	match->cp_next = match->cp_prev = NULL;
    else if (dir == FORWARD)
    {
	match->cp_next = compl_curr_match->cp_next;
	match->cp_prev = compl_curr_match;
    }
    else	 
    {
	match->cp_next = compl_curr_match;
	match->cp_prev = compl_curr_match->cp_prev;
    }
    if (match->cp_next)
	match->cp_next->cp_prev = match;
    if (match->cp_prev)
	match->cp_prev->cp_next = match;
    else	 
	compl_first_match = match;
    compl_curr_match = match;
    if (compl_get_longest && (flags & CP_ORIGINAL_TEXT) == 0)
	ins_compl_longest_match(match);
    return OK;
}