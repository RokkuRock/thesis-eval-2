clear_evalarg(evalarg_T *evalarg, exarg_T *eap)
{
    if (evalarg != NULL)
    {
	if (evalarg->eval_tofree != NULL)
	{
	    if (eap != NULL)
	    {
		vim_free(eap->cmdline_tofree);
		eap->cmdline_tofree = *eap->cmdlinep;
		*eap->cmdlinep = evalarg->eval_tofree;
	    }
	    else
		vim_free(evalarg->eval_tofree);
	    evalarg->eval_tofree = NULL;
	}
	ga_clear_strings(&evalarg->eval_tofree_ga);
	VIM_CLEAR(evalarg->eval_tofree_lambda);
    }
}