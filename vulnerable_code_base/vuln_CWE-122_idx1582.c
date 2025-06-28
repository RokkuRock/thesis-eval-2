get_var_dest(
	char_u		*name,
	assign_dest_T	*dest,
	cmdidx_T	cmdidx,
	int		*option_scope,
	int		*vimvaridx,
	type_T		**type,
	cctx_T		*cctx)
{
    char_u *p;
    if (*name == '&')
    {
	int		cc;
	long		numval;
	getoption_T	opt_type;
	int		opt_p_flags;
	*dest = dest_option;
	if (cmdidx == CMD_final || cmdidx == CMD_const)
	{
	    emsg(_(e_cannot_lock_option));
	    return FAIL;
	}
	p = name;
	p = find_option_end(&p, option_scope);
	if (p == NULL)
	{
	    emsg(_(e_unexpected_characters_in_assignment));
	    return FAIL;
	}
	cc = *p;
	*p = NUL;
	opt_type = get_option_value(skip_option_env_lead(name),
				   &numval, NULL, &opt_p_flags, *option_scope);
	*p = cc;
	switch (opt_type)
	{
	    case gov_unknown:
		    semsg(_(e_unknown_option_str), name);
		    return FAIL;
	    case gov_string:
	    case gov_hidden_string:
		    if (opt_p_flags & P_FUNC)
		    {
			*type = &t_any;
			*dest = dest_func_option;
		    }
		    else
		    {
			*type = &t_string;
		    }
		    break;
	    case gov_bool:
	    case gov_hidden_bool:
		    *type = &t_bool;
		    break;
	    case gov_number:
	    case gov_hidden_number:
		    *type = &t_number;
		    break;
	}
    }
    else if (*name == '$')
    {
	*dest = dest_env;
	*type = &t_string;
    }
    else if (*name == '@')
    {
	if (name[1] != '@'
			&& (!valid_yank_reg(name[1], FALSE) || name[1] == '.'))
	{
	    emsg_invreg(name[1]);
	    return FAIL;
	}
	*dest = dest_reg;
	*type = name[1] == '#' ? &t_number_or_string : &t_string;
    }
    else if (STRNCMP(name, "g:", 2) == 0)
    {
	*dest = dest_global;
    }
    else if (STRNCMP(name, "b:", 2) == 0)
    {
	*dest = dest_buffer;
    }
    else if (STRNCMP(name, "w:", 2) == 0)
    {
	*dest = dest_window;
    }
    else if (STRNCMP(name, "t:", 2) == 0)
    {
	*dest = dest_tab;
    }
    else if (STRNCMP(name, "v:", 2) == 0)
    {
	typval_T	*vtv;
	int		di_flags;
	*vimvaridx = find_vim_var(name + 2, &di_flags);
	if (*vimvaridx < 0)
	{
	    semsg(_(e_variable_not_found_str), name);
	    return FAIL;
	}
	if (var_check_ro(di_flags, name, FALSE))
	    return FAIL;
	*dest = dest_vimvar;
	vtv = get_vim_var_tv(*vimvaridx);
	*type = typval2type_vimvar(vtv, cctx->ctx_type_list);
    }
    return OK;
}