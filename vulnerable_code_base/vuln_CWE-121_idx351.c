ex_finally(exarg_T *eap)
{
    int		idx;
    int		skip = FALSE;
    int		pending = CSTP_NONE;
    cstack_T	*cstack = eap->cstack;
    if (cmdmod_error(FALSE))
	return;
    if (cstack->cs_trylevel <= 0 || cstack->cs_idx < 0)
	eap->errmsg = _(e_finally_without_try);
    else
    {
	if (!(cstack->cs_flags[cstack->cs_idx] & CSF_TRY))
	{
	    eap->errmsg = get_end_emsg(cstack);
	    for (idx = cstack->cs_idx - 1; idx > 0; --idx)
		if (cstack->cs_flags[idx] & CSF_TRY)
		    break;
	    pending = CSTP_ERROR;
	}
	else
	    idx = cstack->cs_idx;
	if (cstack->cs_flags[idx] & CSF_FINALLY)
	{
	    eap->errmsg = _(e_multiple_finally);
	    return;
	}
	rewind_conditionals(cstack, idx, CSF_WHILE | CSF_FOR,
						       &cstack->cs_looplevel);
	skip = !(cstack->cs_flags[cstack->cs_idx] & CSF_TRUE);
	if (!skip)
	{
	    if (dbg_check_skipped(eap))
	    {
		(void)do_intthrow(cstack);
	    }
	    cleanup_conditionals(cstack, CSF_TRY, FALSE);
	    if (cstack->cs_idx >= 0
			       && (cstack->cs_flags[cstack->cs_idx] & CSF_TRY))
	    {
		leave_block(cstack);
		enter_block(cstack);
	    }
	    if (pending == CSTP_ERROR || did_emsg || got_int || did_throw)
	    {
		if (cstack->cs_pending[cstack->cs_idx] == CSTP_RETURN)
		{
		    report_discard_pending(CSTP_RETURN,
					   cstack->cs_rettv[cstack->cs_idx]);
		    discard_pending_return(cstack->cs_rettv[cstack->cs_idx]);
		}
		if (pending == CSTP_ERROR && !did_emsg)
		    pending |= (THROW_ON_ERROR) ? CSTP_THROW : 0;
		else
		    pending |= did_throw ? CSTP_THROW : 0;
		pending |= did_emsg  ? CSTP_ERROR     : 0;
		pending |= got_int   ? CSTP_INTERRUPT : 0;
		cstack->cs_pending[cstack->cs_idx] = pending;
		if (did_throw && cstack->cs_exception[cstack->cs_idx]
							 != current_exception)
		    internal_error("ex_finally()");
	    }
	    cstack->cs_lflags |= CSL_HAD_FINA;
	}
    }
}