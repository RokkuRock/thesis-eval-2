eval_lambda(
    char_u	**arg,
    typval_T	*rettv,
    evalarg_T	*evalarg,
    int		verbose)	 
{
    int		evaluate = evalarg != NULL
				      && (evalarg->eval_flags & EVAL_EVALUATE);
    typval_T	base = *rettv;
    int		ret;
    rettv->v_type = VAR_UNKNOWN;
    if (**arg == '{')
    {
	ret = get_lambda_tv(arg, rettv, FALSE, evalarg);
    }
    else
    {
	++*arg;
	ret = eval1(arg, rettv, evalarg);
	*arg = skipwhite_and_linebreak(*arg, evalarg);
	if (**arg != ')')
	{
	    emsg(_(e_missing_closing_paren));
	    ret = FAIL;
	}
	++*arg;
    }
    if (ret != OK)
	return FAIL;
    else if (**arg != '(')
    {
	if (verbose)
	{
	    if (*skipwhite(*arg) == '(')
		emsg(_(e_nowhitespace));
	    else
		semsg(_(e_missing_parenthesis_str), "lambda");
	}
	clear_tv(rettv);
	ret = FAIL;
    }
    else
	ret = call_func_rettv(arg, evalarg, rettv, evaluate, NULL, &base);
    if (evaluate)
	clear_tv(&base);
    return ret;
}