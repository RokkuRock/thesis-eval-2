compile_nested_function(exarg_T *eap, cctx_T *cctx)
{
    int		is_global = *eap->arg == 'g' && eap->arg[1] == ':';
    char_u	*name_start = eap->arg;
    char_u	*name_end = to_name_end(eap->arg, TRUE);
    char_u	*lambda_name;
    ufunc_T	*ufunc;
    int		r = FAIL;
    compiletype_T   compile_type;
    if (eap->forceit)
    {
	emsg(_(e_cannot_use_bang_with_nested_def));
	return NULL;
    }
    if (*name_start == '/')
    {
	name_end = skip_regexp(name_start + 1, '/', TRUE);
	if (*name_end == '/')
	    ++name_end;
	set_nextcmd(eap, name_end);
    }
    if (name_end == name_start || *skipwhite(name_end) != '(')
    {
	if (!ends_excmd2(name_start, name_end))
	{
	    semsg(_(e_invalid_command_str), eap->cmd);
	    return NULL;
	}
	if (generate_DEF(cctx, name_start, name_end - name_start) == FAIL)
	    return NULL;
	return eap->nextcmd == NULL ? (char_u *)"" : eap->nextcmd;
    }
    if (name_start[1] == ':' && !is_global)
    {
	semsg(_(e_namespace_not_supported_str), name_start);
	return NULL;
    }
    if (check_defined(name_start, name_end - name_start, cctx, FALSE) == FAIL)
	return NULL;
    eap->arg = name_end;
    fill_exarg_from_cctx(eap, cctx);
    eap->forceit = FALSE;
    lambda_name = vim_strsave(get_lambda_name());
    if (lambda_name == NULL)
	return NULL;
    ufunc = define_function(eap, lambda_name);
    if (ufunc == NULL)
    {
	r = eap->skip ? OK : FAIL;
	goto theend;
    }
    if (!is_global && cctx->ctx_ufunc->uf_block_depth > 0)
    {
	int block_depth = cctx->ctx_ufunc->uf_block_depth;
	ufunc->uf_block_ids = ALLOC_MULT(int, block_depth);
	if (ufunc->uf_block_ids != NULL)
	{
	    mch_memmove(ufunc->uf_block_ids, cctx->ctx_ufunc->uf_block_ids,
						    sizeof(int) * block_depth);
	    ufunc->uf_block_depth = block_depth;
	}
    }
    compile_type = COMPILE_TYPE(ufunc);
#ifdef FEAT_PROFILE
    if (cctx->ctx_compile_type == CT_PROFILE)
	compile_type = CT_PROFILE;
#endif
    if (func_needs_compiling(ufunc, compile_type)
	    && compile_def_function(ufunc, TRUE, compile_type, cctx) == FAIL)
    {
	func_ptr_unref(ufunc);
	goto theend;
    }
#ifdef FEAT_PROFILE
    if (compile_type == CT_PROFILE && func_needs_compiling(ufunc, CT_NONE))
	compile_def_function(ufunc, FALSE, CT_NONE, cctx);
#endif
    if (is_global)
    {
	char_u *func_name = vim_strnsave(name_start + 2,
						    name_end - name_start - 2);
	if (func_name == NULL)
	    r = FAIL;
	else
	{
	    r = generate_NEWFUNC(cctx, lambda_name, func_name);
	    lambda_name = NULL;
	}
    }
    else
    {
	lvar_T	*lvar = reserve_local(cctx, name_start, name_end - name_start,
						    TRUE, ufunc->uf_func_type);
	if (lvar == NULL)
	    goto theend;
	if (generate_FUNCREF(cctx, ufunc) == FAIL)
	    goto theend;
	r = generate_STORE(cctx, ISN_STORE, lvar->lv_idx, NULL);
    }
theend:
    vim_free(lambda_name);
    return r == FAIL ? NULL : (char_u *)"";
}