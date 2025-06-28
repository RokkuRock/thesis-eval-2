ex_endtry(exarg_T *eap)
{
    int		idx;
    int		skip;
    int		rethrow = FALSE;
    int		pending = CSTP_NONE;
    void	*rettv = NULL;
    cstack_T	*cstack = eap->cstack;
    if (cmdmod_error(FALSE))
	return;
    if (cstack->cs_trylevel <= 0 || cstack->cs_idx < 0)
	eap->errmsg = _(e_endtry_without_try);
    else
    {
	skip = did_emsg || got_int || did_throw
			     || !(cstack->cs_flags[cstack->cs_idx] & CSF_TRUE);
	if (!(cstack->cs_flags[cstack->cs_idx] & CSF_TRY))
	{
	    eap->errmsg = get_end_emsg(cstack);
	    idx = cstack->cs_idx;
	    do
		--idx;
	    while (idx > 0 && !(cstack->cs_flags[idx] & CSF_TRY));
	    rewind_conditionals(cstack, idx, CSF_WHILE | CSF_FOR,
						       &cstack->cs_looplevel);
	    skip = TRUE;
	    if (did_throw)
		discard_current_exception();
	    did_emsg = FALSE;
	}
	else
	{
	    idx = cstack->cs_idx;
	    if (!skip && in_vim9script()
		     && (cstack->cs_flags[idx] & (CSF_CATCH|CSF_FINALLY)) == 0)
	    {
		eap->errmsg = _(e_missing_catch_or_finally);
	    }
	    if (did_throw && (cstack->cs_flags[idx] & CSF_TRUE)
		    && !(cstack->cs_flags[idx] & CSF_FINALLY))
		rethrow = TRUE;
	}
	if ((rethrow || (!skip
			&& !(cstack->cs_flags[idx] & CSF_FINALLY)
			&& !cstack->cs_pending[idx]))
		&& dbg_check_skipped(eap))
	{
	    if (got_int)
	    {
		skip = TRUE;
		(void)do_intthrow(cstack);
		rethrow = FALSE;
		if (did_throw && !(cstack->cs_flags[idx] & CSF_FINALLY))
		    rethrow = TRUE;
	    }
	}
	if (!skip)
	{
	    pending = cstack->cs_pending[idx];
	    cstack->cs_pending[idx] = CSTP_NONE;
	    if (pending == CSTP_RETURN)
		rettv = cstack->cs_rettv[idx];
	    else if (pending & CSTP_THROW)
		current_exception = cstack->cs_exception[idx];
	}
	(void)cleanup_conditionals(cstack, CSF_TRY | CSF_SILENT, TRUE);
	if (cstack->cs_idx >= 0
			       && (cstack->cs_flags[cstack->cs_idx] & CSF_TRY))
	    leave_block(cstack);
	--cstack->cs_trylevel;
	if (!skip)
	{
	    report_resume_pending(pending,
		    (pending == CSTP_RETURN) ? rettv :
		    (pending & CSTP_THROW) ? (void *)current_exception : NULL);
	    switch (pending)
	    {
		case CSTP_NONE:
		    break;
		case CSTP_CONTINUE:
		    ex_continue(eap);
		    break;
		case CSTP_BREAK:
		    ex_break(eap);
		    break;
		case CSTP_RETURN:
		    do_return(eap, FALSE, FALSE, rettv);
		    break;
		case CSTP_FINISH:
		    do_finish(eap, FALSE);
		    break;
		default:
		    if (pending & CSTP_ERROR)
			did_emsg = TRUE;
		    if (pending & CSTP_INTERRUPT)
			got_int = TRUE;
		    if (pending & CSTP_THROW)
			rethrow = TRUE;
		    break;
	    }
	}
	if (rethrow)
	    do_throw(cstack);
    }
}