get_function_body(
	exarg_T	    *eap,
	garray_T    *newlines,
	char_u	    *line_arg_in,
	char_u	    **line_to_free)
{
    linenr_T	sourcing_lnum_top = SOURCING_LNUM;
    linenr_T	sourcing_lnum_off;
    int		saved_wait_return = need_wait_return;
    char_u	*line_arg = line_arg_in;
    int		vim9_function = eap->cmdidx == CMD_def
						   || eap->cmdidx == CMD_block;
#define MAX_FUNC_NESTING 50
    char	nesting_def[MAX_FUNC_NESTING];
    char	nesting_inline[MAX_FUNC_NESTING];
    int		nesting = 0;
    getline_opt_T getline_options;
    int		indent = 2;
    char_u	*skip_until = NULL;
    int		ret = FAIL;
    int		is_heredoc = FALSE;
    int		heredoc_concat_len = 0;
    garray_T	heredoc_ga;
    char_u	*heredoc_trimmed = NULL;
    ga_init2(&heredoc_ga, 1, 500);
    sourcing_lnum_off = get_sourced_lnum(eap->getline, eap->cookie);
    if (SOURCING_LNUM < sourcing_lnum_off)
    {
	sourcing_lnum_off -= SOURCING_LNUM;
	if (ga_grow(newlines, sourcing_lnum_off) == FAIL)
	    goto theend;
	while (sourcing_lnum_off-- > 0)
	    ((char_u **)(newlines->ga_data))[newlines->ga_len++] = NULL;
    }
    nesting_def[0] = vim9_function;
    nesting_inline[0] = eap->cmdidx == CMD_block;
    getline_options = vim9_function
				? GETLINE_CONCAT_CONTBAR : GETLINE_CONCAT_CONT;
    for (;;)
    {
	char_u	*theline;
	char_u	*p;
	char_u	*arg;
	if (KeyTyped)
	{
	    msg_scroll = TRUE;
	    saved_wait_return = FALSE;
	}
	need_wait_return = FALSE;
	if (line_arg != NULL)
	{
	    theline = line_arg;
	    p = vim_strchr(theline, '\n');
	    if (p == NULL)
		line_arg += STRLEN(line_arg);
	    else
	    {
		*p = NUL;
		line_arg = p + 1;
	    }
	}
	else
	{
	    vim_free(*line_to_free);
	    if (eap->getline == NULL)
		theline = getcmdline(':', 0L, indent, getline_options);
	    else
		theline = eap->getline(':', eap->cookie, indent,
							      getline_options);
	    *line_to_free = theline;
	}
	if (KeyTyped)
	    lines_left = Rows - 1;
	if (theline == NULL)
	{
	    SOURCING_LNUM = sourcing_lnum_top;
	    if (skip_until != NULL)
		semsg(_(e_missing_heredoc_end_marker_str), skip_until);
	    else if (nesting_inline[nesting])
		emsg(_(e_missing_end_block));
	    else if (eap->cmdidx == CMD_def)
		emsg(_(e_missing_enddef));
	    else
		emsg(_("E126: Missing :endfunction"));
	    goto theend;
	}
	sourcing_lnum_off = get_sourced_lnum(eap->getline, eap->cookie);
	if (SOURCING_LNUM < sourcing_lnum_off)
	    sourcing_lnum_off -= SOURCING_LNUM;
	else
	    sourcing_lnum_off = 0;
	if (skip_until != NULL)
	{
	    if (heredoc_trimmed == NULL
		    || (is_heredoc && skipwhite(theline) == theline)
		    || STRNCMP(theline, heredoc_trimmed,
						 STRLEN(heredoc_trimmed)) == 0)
	    {
		if (heredoc_trimmed == NULL)
		    p = theline;
		else if (is_heredoc)
		    p = skipwhite(theline) == theline
				 ? theline : theline + STRLEN(heredoc_trimmed);
		else
		    p = theline + STRLEN(heredoc_trimmed);
		if (STRCMP(p, skip_until) == 0)
		{
		    VIM_CLEAR(skip_until);
		    VIM_CLEAR(heredoc_trimmed);
		    getline_options = vim9_function
				? GETLINE_CONCAT_CONTBAR : GETLINE_CONCAT_CONT;
		    is_heredoc = FALSE;
		    if (heredoc_concat_len > 0)
		    {
			ga_concat(&heredoc_ga, theline);
			vim_free(((char_u **)(newlines->ga_data))[
						      heredoc_concat_len - 1]);
			((char_u **)(newlines->ga_data))[
				  heredoc_concat_len - 1] = heredoc_ga.ga_data;
			ga_init(&heredoc_ga);
			heredoc_concat_len = 0;
			theline += STRLEN(theline);   
		    }
		}
	    }
	}
	else
	{
	    int	    c;
	    char_u  *end;
	    for (p = theline; VIM_ISWHITE(*p) || *p == ':'; ++p)
		;
	    if (nesting_inline[nesting]
		    ? *p == '}'
		    : (checkforcmd(&p, nesting_def[nesting]
						? "enddef" : "endfunction", 4)
			&& *p != ':'))
	    {
		if (nesting-- == 0)
		{
		    char_u *nextcmd = NULL;
		    if (*p == '|' || *p == '}')
			nextcmd = p + 1;
		    else if (line_arg != NULL && *skipwhite(line_arg) != NUL)
			nextcmd = line_arg;
		    else if (*p != NUL && *p != (vim9_function ? '#' : '"')
					   && (vim9_function || p_verbose > 0))
		    {
			SOURCING_LNUM = sourcing_lnum_top
							+ newlines->ga_len + 1;
			if (eap->cmdidx == CMD_def)
			    semsg(_(e_text_found_after_enddef_str), p);
			else
			    give_warning2((char_u *)
				   _("W22: Text found after :endfunction: %s"),
				   p, TRUE);
		    }
		    if (nextcmd != NULL && *skipwhite(nextcmd) != NUL)
		    {
			eap->nextcmd = nextcmd;
			if (*line_to_free != NULL)
			{
			    vim_free(*eap->cmdlinep);
			    *eap->cmdlinep = *line_to_free;
			    *line_to_free = NULL;
			}
		    }
		    break;
		}
	    }
	    else if (nesting_def[nesting])
	    {
		if (checkforcmd(&p, "endfunction", 4) && *p != ':')
		    emsg(_(e_mismatched_endfunction));
	    }
	    else if (eap->cmdidx == CMD_def && checkforcmd(&p, "enddef", 4))
		emsg(_(e_mismatched_enddef));
	    if (indent > 2 && (*p == '}' || STRNCMP(p, "end", 3) == 0))
		indent -= 2;
	    else if (STRNCMP(p, "if", 2) == 0
		    || STRNCMP(p, "wh", 2) == 0
		    || STRNCMP(p, "for", 3) == 0
		    || STRNCMP(p, "try", 3) == 0)
		indent += 2;
	    c = *p;
	    if (is_function_cmd(&p)
		    || (eap->cmdidx == CMD_def && checkforcmd(&p, "def", 3)))
	    {
		if (*p == '!')
		    p = skipwhite(p + 1);
		p += eval_fname_script(p);
		vim_free(trans_function_name(&p, NULL, TRUE, 0, NULL,
								  NULL, NULL));
		if (*skipwhite(p) == '(')
		{
		    if (nesting == MAX_FUNC_NESTING - 1)
			emsg(_(e_function_nesting_too_deep));
		    else
		    {
			++nesting;
			nesting_def[nesting] = (c == 'd');
			nesting_inline[nesting] = FALSE;
			indent += 2;
		    }
		}
	    }
	    if (nesting_def[nesting] ? *p != '#' : *p != '"')
	    {
		end = p + STRLEN(p) - 1;
		while (end > p && VIM_ISWHITE(*end))
		    --end;
		if (end > p + 1 && *end == '{' && VIM_ISWHITE(end[-1]))
		{
		    int	    is_block;
		    --end;
		    while (end > p && VIM_ISWHITE(*end))
			--end;
		    is_block = end > p + 2 && end[-1] == '=' && end[0] == '>';
		    if (!is_block)
		    {
			char_u *s = p;
			is_block = checkforcmd_noparen(&s, "autocmd", 2)
				      || checkforcmd_noparen(&s, "command", 3);
		    }
		    if (is_block)
		    {
			if (nesting == MAX_FUNC_NESTING - 1)
			    emsg(_(e_function_nesting_too_deep));
			else
			{
			    ++nesting;
			    nesting_def[nesting] = TRUE;
			    nesting_inline[nesting] = TRUE;
			    indent += 2;
			}
		    }
		}
	    }
	    p = skip_range(p, FALSE, NULL);
	    if (!vim9_function
		&& ((p[0] == 'a' && (!ASCII_ISALPHA(p[1]) || p[1] == 'p'))
		    || (p[0] == 'c'
			&& (!ASCII_ISALPHA(p[1]) || (p[1] == 'h'
				&& (!ASCII_ISALPHA(p[2]) || (p[2] == 'a'
					&& (STRNCMP(&p[3], "nge", 3) != 0
					    || !ASCII_ISALPHA(p[6])))))))
		    || (p[0] == 'i'
			&& (!ASCII_ISALPHA(p[1]) || (p[1] == 'n'
				&& (!ASCII_ISALPHA(p[2])
				    || (p[2] == 's'
					&& (!ASCII_ISALPHA(p[3])
						|| p[3] == 'e'))))))))
		skip_until = vim_strsave((char_u *)".");
	    arg = skipwhite(skiptowhite(p));
	    if (arg[0] == '<' && arg[1] =='<'
		    && ((p[0] == 'p' && p[1] == 'y'
				    && (!ASCII_ISALNUM(p[2]) || p[2] == 't'
					|| ((p[2] == '3' || p[2] == 'x')
						   && !ASCII_ISALPHA(p[3]))))
			|| (p[0] == 'p' && p[1] == 'e'
				    && (!ASCII_ISALPHA(p[2]) || p[2] == 'r'))
			|| (p[0] == 't' && p[1] == 'c'
				    && (!ASCII_ISALPHA(p[2]) || p[2] == 'l'))
			|| (p[0] == 'l' && p[1] == 'u' && p[2] == 'a'
				    && !ASCII_ISALPHA(p[3]))
			|| (p[0] == 'r' && p[1] == 'u' && p[2] == 'b'
				    && (!ASCII_ISALPHA(p[3]) || p[3] == 'y'))
			|| (p[0] == 'm' && p[1] == 'z'
				    && (!ASCII_ISALPHA(p[2]) || p[2] == 's'))
			))
	    {
		p = skipwhite(arg + 2);
		if (STRNCMP(p, "trim", 4) == 0)
		{
		    p = skipwhite(p + 4);
		    heredoc_trimmed = vim_strnsave(theline,
						 skipwhite(theline) - theline);
		}
		if (*p == NUL)
		    skip_until = vim_strsave((char_u *)".");
		else
		    skip_until = vim_strnsave(p, skiptowhite(p) - p);
		getline_options = GETLINE_NONE;
		is_heredoc = TRUE;
		if (eap->cmdidx == CMD_def)
		    heredoc_concat_len = newlines->ga_len + 1;
	    }
	    arg = skipwhite(skiptowhite(p));
	    if (*arg == '[')
		arg = vim_strchr(arg, ']');
	    if (arg != NULL)
	    {
		int found = (eap->cmdidx == CMD_def && arg[0] == '='
					     && arg[1] == '<' && arg[2] =='<');
		if (!found)
		    arg = skipwhite(skiptowhite(arg));
		if (found || (arg[0] == '=' && arg[1] == '<' && arg[2] =='<'
			&& (checkforcmd(&p, "let", 2)
			    || checkforcmd(&p, "var", 3)
			    || checkforcmd(&p, "final", 5)
			    || checkforcmd(&p, "const", 5))))
		{
		    p = skipwhite(arg + 3);
		    if (STRNCMP(p, "trim", 4) == 0)
		    {
			p = skipwhite(p + 4);
			heredoc_trimmed = vim_strnsave(theline,
						 skipwhite(theline) - theline);
		    }
		    skip_until = vim_strnsave(p, skiptowhite(p) - p);
		    getline_options = GETLINE_NONE;
		    is_heredoc = TRUE;
		}
	    }
	}
	if (ga_grow(newlines, 1 + sourcing_lnum_off) == FAIL)
	    goto theend;
	if (heredoc_concat_len > 0)
	{
	    ga_concat(&heredoc_ga, theline);
	    ga_concat(&heredoc_ga, (char_u *)"\n");
	    p = vim_strsave((char_u *)"");
	}
	else
	{
	    p = vim_strsave(theline);
	}
	if (p == NULL)
	    goto theend;
	((char_u **)(newlines->ga_data))[newlines->ga_len++] = p;
	while (sourcing_lnum_off-- > 0)
	    ((char_u **)(newlines->ga_data))[newlines->ga_len++] = NULL;
	if (line_arg != NULL && *line_arg == NUL)
	    line_arg = NULL;
    }
    if (!did_emsg)
	ret = OK;
theend:
    vim_free(skip_until);
    vim_free(heredoc_trimmed);
    vim_free(heredoc_ga.ga_data);
    need_wait_return |= saved_wait_return;
    return ret;
}