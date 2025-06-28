skip_expr_concatenate(
	char_u	    **arg,
	char_u	    **start,
	char_u	    **end,
	evalarg_T   *evalarg)
{
    typval_T	rettv;
    int		res;
    int		vim9script = in_vim9script();
    garray_T    *gap = evalarg == NULL ? NULL : &evalarg->eval_ga;
    garray_T    *freegap = evalarg == NULL ? NULL : &evalarg->eval_freega;
    int		save_flags = evalarg == NULL ? 0 : evalarg->eval_flags;
    int		evaluate = evalarg == NULL
			       ? FALSE : (evalarg->eval_flags & EVAL_EVALUATE);
    if (vim9script && evaluate
	       && (evalarg->eval_cookie != NULL || evalarg->eval_cctx != NULL))
    {
	ga_init2(gap, sizeof(char_u *), 10);
	if (ga_grow(gap, 1) == OK)
	    ++gap->ga_len;
	ga_init2(freegap, sizeof(char_u *), 10);
    }
    *start = *arg;
    if (evalarg != NULL)
	evalarg->eval_flags &= ~EVAL_EVALUATE;
    *arg = skipwhite(*arg);
    res = eval1(arg, &rettv, evalarg);
    *end = *arg;
    if (evalarg != NULL)
	evalarg->eval_flags = save_flags;
    if (vim9script && evaluate
	    && (evalarg->eval_cookie != NULL || evalarg->eval_cctx != NULL))
    {
	if (evalarg->eval_ga.ga_len == 1)
	{
	    ga_clear(gap);
	    gap->ga_itemsize = 0;
	}
	else
	{
	    char_u	    *p;
	    size_t	    endoff = STRLEN(*arg);
	    *((char_u **)gap->ga_data) = *start;
	    p = ga_concat_strings(gap, " ");
	    if (evalarg->eval_cookie != NULL)
	    {
		*((char_u **)gap->ga_data) = NULL;
		vim_free(evalarg->eval_tofree);
		evalarg->eval_tofree =
				    ((char_u **)gap->ga_data)[gap->ga_len - 1];
		((char_u **)gap->ga_data)[gap->ga_len - 1] = NULL;
		ga_clear_strings(gap);
	    }
	    else
	    {
		ga_clear(gap);
		ga_clear_strings(freegap);
	    }
	    gap->ga_itemsize = 0;
	    if (p == NULL)
		return FAIL;
	    *start = p;
	    vim_free(evalarg->eval_tofree_lambda);
	    evalarg->eval_tofree_lambda = p;
	    *end = *start + STRLEN(*start) - endoff;
	}
    }
    return res;
}