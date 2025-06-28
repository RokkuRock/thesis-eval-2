getout(int exitval)
{
    exiting = TRUE;
#if defined(FEAT_EVAL)
    ch_log(NULL, "Exiting...");
#endif
    if (exmode_active)
	exitval += ex_exitval;
#ifdef FEAT_EVAL
    set_vim_var_type(VV_EXITING, VAR_NUMBER);
    set_vim_var_nr(VV_EXITING, exitval);
#endif
    if (!is_not_a_term_or_gui())
	windgoto((int)Rows - 1, 0);
#ifdef FEAT_EVAL
    invoke_all_defer();
#endif
#if defined(FEAT_EVAL) || defined(FEAT_SYN_HL)
    hash_debug_results();
#endif
#ifdef FEAT_GUI
    msg_didany = FALSE;
#endif
    if (v_dying <= 1)
    {
	tabpage_T	*tp;
	tabpage_T	*next_tp;
	buf_T		*buf;
	win_T		*wp;
	int		unblock = 0;
	for (tp = first_tabpage; tp != NULL; tp = next_tp)
	{
	    next_tp = tp->tp_next;
	    FOR_ALL_WINDOWS_IN_TAB(tp, wp)
	    {
		if (wp->w_buffer == NULL)
		    continue;
		buf = wp->w_buffer;
		if (CHANGEDTICK(buf) != -1)
		{
		    bufref_T bufref;
		    set_bufref(&bufref, buf);
		    apply_autocmds(EVENT_BUFWINLEAVE, buf->b_fname,
						    buf->b_fname, FALSE, buf);
		    if (bufref_valid(&bufref))
			CHANGEDTICK(buf) = -1;   
		    next_tp = first_tabpage;
		    break;
		}
	    }
	}
	FOR_ALL_BUFFERS(buf)
	    if (buf->b_ml.ml_mfp != NULL)
	    {
		bufref_T bufref;
		set_bufref(&bufref, buf);
		apply_autocmds(EVENT_BUFUNLOAD, buf->b_fname, buf->b_fname,
								  FALSE, buf);
		if (!bufref_valid(&bufref))
		    break;
	    }
	if (is_autocmd_blocked())
	{
	    unblock_autocmds();
	    ++unblock;
	}
	apply_autocmds(EVENT_VIMLEAVEPRE, NULL, NULL, FALSE, curbuf);
	if (unblock)
	    block_autocmds();
    }
#ifdef FEAT_VIMINFO
    if (*p_viminfo != NUL)
	write_viminfo(NULL, FALSE);
#endif
    if (v_dying <= 1)
    {
	int	unblock = 0;
	if (is_autocmd_blocked())
	{
	    unblock_autocmds();
	    ++unblock;
	}
	apply_autocmds(EVENT_VIMLEAVE, NULL, NULL, FALSE, curbuf);
	if (unblock)
	    block_autocmds();
    }
#ifdef FEAT_PROFILE
    profile_dump();
#endif
    if (did_emsg
#ifdef FEAT_GUI
	    || (gui.in_use && msg_didany && p_verbose > 0)
#endif
	    )
    {
	no_wait_return = FALSE;
	wait_return(FALSE);
    }
    if (!is_not_a_term_or_gui())
	windgoto((int)Rows - 1, 0);
#ifdef FEAT_JOB_CHANNEL
    job_stop_on_exit();
#endif
#ifdef FEAT_LUA
    lua_end();
#endif
#ifdef FEAT_MZSCHEME
    mzscheme_end();
#endif
#ifdef FEAT_TCL
    tcl_end();
#endif
#ifdef FEAT_RUBY
    ruby_end();
#endif
#ifdef FEAT_PYTHON
    python_end();
#endif
#ifdef FEAT_PYTHON3
    python3_end();
#endif
#ifdef FEAT_PERL
    perl_end();
#endif
#if defined(USE_ICONV) && defined(DYNAMIC_ICONV)
    iconv_end();
#endif
#ifdef FEAT_NETBEANS_INTG
    netbeans_end();
#endif
#ifdef FEAT_CSCOPE
    cs_end();
#endif
#ifdef FEAT_EVAL
    if (garbage_collect_at_exit)
	garbage_collect(FALSE);
#endif
#ifdef MSWIN
    free_cmd_argsW();
#endif
    mch_exit(exitval);
}