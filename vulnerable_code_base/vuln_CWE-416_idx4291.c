define_function(exarg_T *eap, char_u *name_arg, char_u **line_to_free)
{
    int		j;
    int		c;
    int		saved_did_emsg;
    char_u	*name = name_arg;
    int		is_global = FALSE;
    char_u	*p;
    char_u	*arg;
    char_u	*whitep;
    char_u	*line_arg = NULL;
    garray_T	newargs;
    garray_T	argtypes;
    garray_T	default_args;
    garray_T	newlines;
    int		varargs = FALSE;
    int		flags = 0;
    char_u	*ret_type = NULL;
    ufunc_T	*fp = NULL;
    int		fp_allocated = FALSE;
    int		free_fp = FALSE;
    int		overwrite = FALSE;
    dictitem_T	*v;
    funcdict_T	fudi;
    static int	func_nr = 0;	     
    int		paren;
    hashitem_T	*hi;
    linenr_T	sourcing_lnum_top;
    int		vim9script = in_vim9script();
    imported_T	*import = NULL;
    if (ends_excmd2(eap->cmd, eap->arg))
    {
	if (!eap->skip)
	    list_functions(NULL);
	set_nextcmd(eap, eap->arg);
	return NULL;
    }
    if (*eap->arg == '/')
    {
	p = skip_regexp(eap->arg + 1, '/', TRUE);
	if (!eap->skip)
	{
	    regmatch_T	regmatch;
	    c = *p;
	    *p = NUL;
	    regmatch.regprog = vim_regcomp(eap->arg + 1, RE_MAGIC);
	    *p = c;
	    if (regmatch.regprog != NULL)
	    {
		regmatch.rm_ic = p_ic;
		list_functions(&regmatch);
		vim_regfree(regmatch.regprog);
	    }
	}
	if (*p == '/')
	    ++p;
	set_nextcmd(eap, p);
	return NULL;
    }
    ga_init(&newargs);
    ga_init(&argtypes);
    ga_init(&default_args);
    p = eap->arg;
    if (name_arg != NULL)
    {
	paren = TRUE;
	CLEAR_FIELD(fudi);
    }
    else
    {
	name = save_function_name(&p, &is_global, eap->skip,
						       TFN_NO_AUTOLOAD, &fudi);
	paren = (vim_strchr(p, '(') != NULL);
	if (name == NULL && (fudi.fd_dict == NULL || !paren) && !eap->skip)
	{
	    if (!aborting())
	    {
		if (!eap->skip && fudi.fd_newkey != NULL)
		    semsg(_(e_key_not_present_in_dictionary), fudi.fd_newkey);
		vim_free(fudi.fd_newkey);
		return NULL;
	    }
	    else
		eap->skip = TRUE;
	}
    }
    saved_did_emsg = did_emsg;
    did_emsg = FALSE;
    if (!paren)
    {
	if (!ends_excmd(*skipwhite(p)))
	{
	    semsg(_(e_trailing_characters_str), p);
	    goto ret_free;
	}
	set_nextcmd(eap, p);
	if (eap->nextcmd != NULL)
	    *p = NUL;
	if (!eap->skip && !got_int)
	{
	    fp = find_func(name, is_global, NULL);
	    if (fp == NULL && ASCII_ISUPPER(*eap->arg))
	    {
		char_u *up = untrans_function_name(name);
		if (up != NULL)
		    fp = find_func(up, FALSE, NULL);
	    }
	    if (fp != NULL)
	    {
		list_func_head(fp, TRUE);
		for (j = 0; j < fp->uf_lines.ga_len && !got_int; ++j)
		{
		    if (FUNCLINE(fp, j) == NULL)
			continue;
		    msg_putchar('\n');
		    msg_outnum((long)(j + 1));
		    if (j < 9)
			msg_putchar(' ');
		    if (j < 99)
			msg_putchar(' ');
		    msg_prt_line(FUNCLINE(fp, j), FALSE);
		    out_flush();	 
		    ui_breakcheck();
		}
		if (!got_int)
		{
		    msg_putchar('\n');
		    if (fp->uf_def_status != UF_NOT_COMPILED)
			msg_puts("   enddef");
		    else
			msg_puts("   endfunction");
		}
	    }
	    else
		emsg_funcname(e_undefined_function_str, eap->arg);
	}
	goto ret_free;
    }
    p = skipwhite(p);
    if (*p != '(')
    {
	if (!eap->skip)
	{
	    semsg(_(e_missing_paren_str), eap->arg);
	    goto ret_free;
	}
	if (vim_strchr(p, '(') != NULL)
	    p = vim_strchr(p, '(');
    }
    if ((vim9script || eap->cmdidx == CMD_def) && VIM_ISWHITE(p[-1]))
    {
	semsg(_(e_no_white_space_allowed_before_str_str), "(", p - 1);
	goto ret_free;
    }
    if (vim9script && eap->forceit && !is_global)
    {
	emsg(_(e_no_bang_allowed));
	goto ret_free;
    }
    ga_init2(&newlines, (int)sizeof(char_u *), 10);
    if (!eap->skip && name_arg == NULL)
    {
	if (name != NULL)
	    arg = name;
	else
	    arg = fudi.fd_newkey;
	if (arg != NULL && (fudi.fd_di == NULL
				     || (fudi.fd_di->di_tv.v_type != VAR_FUNC
				 && fudi.fd_di->di_tv.v_type != VAR_PARTIAL)))
	{
	    char_u  *name_base = arg;
	    int	    i;
	    if (*arg == K_SPECIAL)
	    {
		name_base = vim_strchr(arg, '_');
		if (name_base == NULL)
		    name_base = arg + 3;
		else
		    ++name_base;
	    }
	    for (i = 0; name_base[i] != NUL && (i == 0
					? eval_isnamec1(name_base[i])
					: eval_isnamec(name_base[i])); ++i)
		;
	    if (name_base[i] != NUL)
		emsg_funcname(e_invalid_argument_str, arg);
	    if (vim9script && *arg == K_SPECIAL
		&& eval_variable(name_base, (int)STRLEN(name_base), 0, NULL,
		    NULL, EVAL_VAR_NOAUTOLOAD + EVAL_VAR_IMPORT
						     + EVAL_VAR_NO_FUNC) == OK)
	    {
		semsg(_(e_redefining_script_item_str), name_base);
		goto ret_free;
	    }
	}
	if (fudi.fd_dict != NULL && fudi.fd_dict->dv_scope == VAR_DEF_SCOPE)
	{
	    emsg(_(e_cannot_use_g_here));
	    goto ret_free;
	}
    }
    ++p;
    if (get_function_args(&p, ')', &newargs,
			eap->cmdidx == CMD_def ? &argtypes : NULL, FALSE,
			 NULL, &varargs, &default_args, eap->skip,
			 eap, line_to_free) == FAIL)
	goto errret_2;
    whitep = p;
    if (eap->cmdidx == CMD_def)
    {
	if (*skipwhite(p) == ':')
	{
	    if (*p != ':')
	    {
		semsg(_(e_no_white_space_allowed_before_colon_str), p);
		p = skipwhite(p);
	    }
	    else if (!IS_WHITE_OR_NUL(p[1]))
		semsg(_(e_white_space_required_after_str_str), ":", p);
	    ret_type = skipwhite(p + 1);
	    p = skip_type(ret_type, FALSE);
	    if (p > ret_type)
	    {
		ret_type = vim_strnsave(ret_type, p - ret_type);
		whitep = p;
		p = skipwhite(p);
	    }
	    else
	    {
		semsg(_(e_expected_type_str), ret_type);
		ret_type = NULL;
	    }
	}
	p = skipwhite(p);
    }
    else
	for (;;)
	{
	    whitep = p;
	    p = skipwhite(p);
	    if (STRNCMP(p, "range", 5) == 0)
	    {
		flags |= FC_RANGE;
		p += 5;
	    }
	    else if (STRNCMP(p, "dict", 4) == 0)
	    {
		flags |= FC_DICT;
		p += 4;
	    }
	    else if (STRNCMP(p, "abort", 5) == 0)
	    {
		flags |= FC_ABORT;
		p += 5;
	    }
	    else if (STRNCMP(p, "closure", 7) == 0)
	    {
		flags |= FC_CLOSURE;
		p += 7;
		if (current_funccal == NULL)
		{
		    emsg_funcname(e_closure_function_should_not_be_at_top_level,
			    name == NULL ? (char_u *)"" : name);
		    goto erret;
		}
	    }
	    else
		break;
	}
    if (*p == '\n')
	line_arg = p + 1;
    else if (*p != NUL
	    && !(*p == '"' && (!vim9script || eap->cmdidx == CMD_function)
						     && eap->cmdidx != CMD_def)
	    && !(VIM_ISWHITE(*whitep) && *p == '#'
				     && (vim9script || eap->cmdidx == CMD_def))
	    && !eap->skip
	    && !did_emsg)
	semsg(_(e_trailing_characters_str), p);
    if (KeyTyped)
    {
	if (!eap->skip && !eap->forceit)
	{
	    if (fudi.fd_dict != NULL && fudi.fd_newkey == NULL)
		emsg(_(e_dictionary_entry_already_exists));
	    else if (name != NULL && find_func(name, is_global, NULL) != NULL)
		emsg_funcname(e_function_str_already_exists_add_bang_to_replace, name);
	}
	if (!eap->skip && did_emsg)
	    goto erret;
	msg_putchar('\n');	     
	cmdline_row = msg_row;
    }
    sourcing_lnum_top = SOURCING_LNUM;
    if (get_function_body(eap, &newlines, line_arg, line_to_free) == FAIL
	    || eap->skip)
	goto erret;
    if (fudi.fd_dict == NULL)
    {
	hashtab_T	*ht;
	v = find_var(name, &ht, TRUE);
	if (v != NULL && v->di_tv.v_type == VAR_FUNC)
	{
	    emsg_funcname(e_function_name_conflicts_with_variable_str, name);
	    goto erret;
	}
	fp = find_func_even_dead(name, is_global, NULL);
	if (vim9script)
	{
	    char_u *uname = untrans_function_name(name);
	    import = find_imported(uname == NULL ? name : uname, 0, NULL);
	}
	if (fp != NULL || import != NULL)
	{
	    int dead = fp != NULL && (fp->uf_flags & FC_DEAD);
	    if (import != NULL
		    || (!dead && !eap->forceit
			&& (fp->uf_script_ctx.sc_sid != current_sctx.sc_sid
			  || fp->uf_script_ctx.sc_seq == current_sctx.sc_seq)))
	    {
		SOURCING_LNUM = sourcing_lnum_top;
		if (vim9script)
		    emsg_funcname(e_name_already_defined_str, name);
		else
		    emsg_funcname(e_function_str_already_exists_add_bang_to_replace, name);
		goto erret;
	    }
	    if (fp->uf_calls > 0)
	    {
		emsg_funcname(
			    e_cannot_redefine_function_str_it_is_in_use, name);
		goto erret;
	    }
	    if (fp->uf_refcount > 1)
	    {
		--fp->uf_refcount;
		fp->uf_flags |= FC_REMOVED;
		fp = NULL;
		overwrite = TRUE;
	    }
	    else
	    {
		char_u *exp_name = fp->uf_name_exp;
		VIM_CLEAR(name);
		fp->uf_name_exp = NULL;
		func_clear_items(fp);
		fp->uf_name_exp = exp_name;
		fp->uf_flags &= ~FC_DEAD;
#ifdef FEAT_PROFILE
		fp->uf_profiling = FALSE;
		fp->uf_prof_initialized = FALSE;
#endif
		fp->uf_def_status = UF_NOT_COMPILED;
	    }
	}
    }
    else
    {
	char	numbuf[20];
	fp = NULL;
	if (fudi.fd_newkey == NULL && !eap->forceit)
	{
	    emsg(_(e_dictionary_entry_already_exists));
	    goto erret;
	}
	if (fudi.fd_di == NULL)
	{
	    if (value_check_lock(fudi.fd_dict->dv_lock, eap->arg, FALSE))
		goto erret;
	}
	else if (value_check_lock(fudi.fd_di->di_tv.v_lock, eap->arg, FALSE))
	    goto erret;
	vim_free(name);
	sprintf(numbuf, "%d", ++func_nr);
	name = vim_strsave((char_u *)numbuf);
	if (name == NULL)
	    goto erret;
    }
    if (fp == NULL)
    {
	if (fudi.fd_dict == NULL && vim_strchr(name, AUTOLOAD_CHAR) != NULL)
	{
	    int	    slen, plen;
	    char_u  *scriptname;
	    j = FAIL;
	    if (SOURCING_NAME != NULL)
	    {
		scriptname = autoload_name(name);
		if (scriptname != NULL)
		{
		    p = vim_strchr(scriptname, '/');
		    plen = (int)STRLEN(p);
		    slen = (int)STRLEN(SOURCING_NAME);
		    if (slen > plen && fnamecmp(p,
					    SOURCING_NAME + slen - plen) == 0)
			j = OK;
		    vim_free(scriptname);
		}
	    }
	    if (j == FAIL)
	    {
		linenr_T save_lnum = SOURCING_LNUM;
		SOURCING_LNUM = sourcing_lnum_top;
		semsg(_(e_function_name_does_not_match_script_file_name_str),
									 name);
		SOURCING_LNUM = save_lnum;
		goto erret;
	    }
	}
	fp = alloc_clear(offsetof(ufunc_T, uf_name) + STRLEN(name) + 1);
	if (fp == NULL)
	    goto erret;
	fp_allocated = TRUE;
	if (fudi.fd_dict != NULL)
	{
	    if (fudi.fd_di == NULL)
	    {
		fudi.fd_di = dictitem_alloc(fudi.fd_newkey);
		if (fudi.fd_di == NULL)
		{
		    vim_free(fp);
		    fp = NULL;
		    goto erret;
		}
		if (dict_add(fudi.fd_dict, fudi.fd_di) == FAIL)
		{
		    vim_free(fudi.fd_di);
		    vim_free(fp);
		    fp = NULL;
		    goto erret;
		}
	    }
	    else
		clear_tv(&fudi.fd_di->di_tv);
	    fudi.fd_di->di_tv.v_type = VAR_FUNC;
	    fudi.fd_di->di_tv.vval.v_string = vim_strsave(name);
	    flags |= FC_DICT;
	}
    }
    fp->uf_args = newargs;
    fp->uf_def_args = default_args;
    fp->uf_ret_type = &t_any;
    fp->uf_func_type = &t_func_any;
    if (eap->cmdidx == CMD_def)
    {
	int	    lnum_save = SOURCING_LNUM;
	cstack_T    *cstack = eap->cstack;
	fp->uf_def_status = UF_TO_BE_COMPILED;
	SOURCING_LNUM = sourcing_lnum_top;
	function_using_block_scopes(fp, cstack);
	if (parse_argument_types(fp, &argtypes, varargs) == FAIL)
	{
	    SOURCING_LNUM = lnum_save;
	    free_fp = fp_allocated;
	    goto erret;
	}
	varargs = FALSE;
	if (parse_return_type(fp, ret_type) == FAIL)
	{
	    SOURCING_LNUM = lnum_save;
	    free_fp = fp_allocated;
	    goto erret;
	}
	SOURCING_LNUM = lnum_save;
    }
    else
	fp->uf_def_status = UF_NOT_COMPILED;
    if (fp_allocated)
    {
	set_ufunc_name(fp, name);
	if (overwrite)
	{
	    hi = hash_find(&func_hashtab, name);
	    hi->hi_key = UF2HIKEY(fp);
	}
	else if (hash_add(&func_hashtab, UF2HIKEY(fp)) == FAIL)
	{
	    free_fp = TRUE;
	    goto erret;
	}
	fp->uf_refcount = 1;
    }
    fp->uf_lines = newlines;
    newlines.ga_data = NULL;
    if ((flags & FC_CLOSURE) != 0)
    {
	if (register_closure(fp) == FAIL)
	    goto erret;
    }
    else
	fp->uf_scoped = NULL;
#ifdef FEAT_PROFILE
    if (prof_def_func())
	func_do_profile(fp);
#endif
    fp->uf_varargs = varargs;
    if (sandbox)
	flags |= FC_SANDBOX;
    if (vim9script && !ASCII_ISUPPER(*fp->uf_name))
	flags |= FC_VIM9;
    fp->uf_flags = flags;
    fp->uf_calls = 0;
    fp->uf_cleared = FALSE;
    fp->uf_script_ctx = current_sctx;
    fp->uf_script_ctx_version = current_sctx.sc_version;
    fp->uf_script_ctx.sc_lnum += sourcing_lnum_top;
    if (is_export)
    {
	fp->uf_flags |= FC_EXPORT;
	is_export = FALSE;
    }
    if (eap->cmdidx == CMD_def)
	set_function_type(fp);
    else if (fp->uf_script_ctx.sc_version == SCRIPT_VERSION_VIM9)
	fp->uf_script_ctx.sc_version = SCRIPT_VERSION_MAX;
    goto ret_free;
erret:
    ga_clear_strings(&newargs);
    ga_clear_strings(&default_args);
    if (fp != NULL)
    {
	ga_init(&fp->uf_args);
	ga_init(&fp->uf_def_args);
    }
errret_2:
    ga_clear_strings(&newlines);
    if (fp != NULL)
	VIM_CLEAR(fp->uf_arg_types);
    if (free_fp)
    {
	vim_free(fp);
	fp = NULL;
    }
ret_free:
    ga_clear_strings(&argtypes);
    vim_free(fudi.fd_newkey);
    if (name != name_arg)
	vim_free(name);
    vim_free(ret_type);
    did_emsg |= saved_did_emsg;
    return fp;
}