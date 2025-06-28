get_function_args(
    char_u	**argp,
    char_u	endchar,
    garray_T	*newargs,
    garray_T	*argtypes,	 
    int		types_optional,	 
    evalarg_T	*evalarg,	 
    int		*varargs,
    garray_T	*default_args,
    int		skip,
    exarg_T	*eap,
    char_u	**line_to_free)
{
    int		mustend = FALSE;
    char_u	*arg;
    char_u	*p;
    int		c;
    int		any_default = FALSE;
    char_u	*expr;
    char_u	*whitep = *argp;
    if (newargs != NULL)
	ga_init2(newargs, (int)sizeof(char_u *), 3);
    if (argtypes != NULL)
	ga_init2(argtypes, (int)sizeof(char_u *), 3);
    if (!skip && default_args != NULL)
	ga_init2(default_args, (int)sizeof(char_u *), 3);
    if (varargs != NULL)
	*varargs = FALSE;
    arg = skipwhite(*argp);
    p = arg;
    while (*p != endchar)
    {
	while (eap != NULL && eap->getline != NULL
			 && (*p == NUL || (VIM_ISWHITE(*whitep) && *p == '#')))
	{
	    char_u *theline = get_function_line(eap, line_to_free, 0,
							  GETLINE_CONCAT_CONT);
	    if (theline == NULL)
		break;
	    whitep = (char_u *)" ";
	    p = skipwhite(theline);
	}
	if (mustend && *p != endchar)
	{
	    if (!skip)
		semsg(_(e_invalid_argument_str), *argp);
	    goto err_ret;
	}
	if (*p == endchar)
	    break;
	if (p[0] == '.' && p[1] == '.' && p[2] == '.')
	{
	    if (varargs != NULL)
		*varargs = TRUE;
	    p += 3;
	    mustend = TRUE;
	    if (argtypes != NULL)
	    {
		if (!eval_isnamec1(*p))
		{
		    if (!skip)
			emsg(_(e_missing_name_after_dots));
		    goto err_ret;
		}
		arg = p;
		p = one_function_arg(p, newargs, argtypes, types_optional,
							  evalarg, TRUE, skip);
		if (p == arg)
		    break;
		if (*skipwhite(p) == '=')
		{
		    emsg(_(e_cannot_use_default_for_variable_arguments));
		    break;
		}
	    }
	}
	else
	{
	    char_u *np;
	    arg = p;
	    p = one_function_arg(p, newargs, argtypes, types_optional,
							 evalarg, FALSE, skip);
	    if (p == arg)
		break;
	    np = skipwhite(p);
	    if (*np == '=' && np[1] != '=' && np[1] != '~'
						       && default_args != NULL)
	    {
		typval_T	rettv;
		any_default = TRUE;
		p = skipwhite(p) + 1;
		whitep = p;
		p = skipwhite(p);
		expr = p;
		if (eval1(&p, &rettv, NULL) != FAIL)
		{
		    if (!skip)
		    {
			if (ga_grow(default_args, 1) == FAIL)
			    goto err_ret;
			while (p > expr && VIM_ISWHITE(p[-1]))
			    p--;
			c = *p;
			*p = NUL;
			expr = vim_strsave(expr);
			if (expr == NULL)
			{
			    *p = c;
			    goto err_ret;
			}
			((char_u **)(default_args->ga_data))
						 [default_args->ga_len] = expr;
			default_args->ga_len++;
			*p = c;
		    }
		}
		else
		    mustend = TRUE;
	    }
	    else if (any_default)
	    {
		emsg(_(e_non_default_argument_follows_default_argument));
		goto err_ret;
	    }
	    if (VIM_ISWHITE(*p) && *skipwhite(p) == ',')
	    {
		if (!skip)
		{
		    semsg(_(e_no_white_space_allowed_before_str_str), ",", p);
		    goto err_ret;
		}
		p = skipwhite(p);
	    }
	    if (*p == ',')
	    {
		++p;
		if (!skip && argtypes != NULL
				      && !IS_WHITE_OR_NUL(*p) && *p != endchar)
		{
		    semsg(_(e_white_space_required_after_str_str), ",", p - 1);
		    goto err_ret;
		}
	    }
	    else
		mustend = TRUE;
	}
	whitep = p;
	p = skipwhite(p);
    }
    if (*p != endchar)
	goto err_ret;
    ++p;	 
    *argp = p;
    return OK;
err_ret:
    if (newargs != NULL)
	ga_clear_strings(newargs);
    if (!skip && default_args != NULL)
	ga_clear_strings(default_args);
    return FAIL;
}