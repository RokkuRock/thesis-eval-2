apply_autocmds_group(
    event_T	event,
    char_u	*fname,	      
    char_u	*fname_io,    
    int		force,	      
    int		group,	      
    buf_T	*buf,	      
    exarg_T	*eap UNUSED)  
{
    char_u	*sfname = NULL;	 
    char_u	*tail;
    int		save_changed;
    buf_T	*old_curbuf;
    int		retval = FALSE;
    char_u	*save_autocmd_fname;
    int		save_autocmd_fname_full;
    int		save_autocmd_bufnr;
    char_u	*save_autocmd_match;
    int		save_autocmd_busy;
    int		save_autocmd_nested;
    static int	nesting = 0;
    AutoPatCmd_T patcmd;
    AutoPat	*ap;
    sctx_T	save_current_sctx;
#ifdef FEAT_EVAL
    funccal_entry_T funccal_entry;
    char_u	*save_cmdarg;
    long	save_cmdbang;
#endif
    static int	filechangeshell_busy = FALSE;
#ifdef FEAT_PROFILE
    proftime_T	wait_time;
#endif
    int		did_save_redobuff = FALSE;
    save_redo_T	save_redo;
    int		save_KeyTyped = KeyTyped;
    int		save_did_emsg;
    ESTACK_CHECK_DECLARATION
    if (event == NUM_EVENTS || first_autopat[(int)event] == NULL
	    || autocmd_blocked > 0)
	goto BYPASS_AU;
    if (autocmd_busy && !(force || autocmd_nested))
	goto BYPASS_AU;
#ifdef FEAT_EVAL
    if (aborting())
	goto BYPASS_AU;
#endif
    if (filechangeshell_busy && (event == EVENT_FILECHANGEDSHELL
				      || event == EVENT_FILECHANGEDSHELLPOST))
	goto BYPASS_AU;
    if (event_ignored(event))
	goto BYPASS_AU;
    if (nesting == 10)
    {
	emsg(_(e_autocommand_nesting_too_deep));
	goto BYPASS_AU;
    }
    if (       (autocmd_no_enter
		&& (event == EVENT_WINENTER || event == EVENT_BUFENTER))
	    || (autocmd_no_leave
		&& (event == EVENT_WINLEAVE || event == EVENT_BUFLEAVE)))
	goto BYPASS_AU;
    save_autocmd_fname = autocmd_fname;
    save_autocmd_fname_full = autocmd_fname_full;
    save_autocmd_bufnr = autocmd_bufnr;
    save_autocmd_match = autocmd_match;
    save_autocmd_busy = autocmd_busy;
    save_autocmd_nested = autocmd_nested;
    save_changed = curbuf->b_changed;
    old_curbuf = curbuf;
    if (fname_io == NULL)
    {
	if (event == EVENT_COLORSCHEME || event == EVENT_COLORSCHEMEPRE
						   || event == EVENT_OPTIONSET
						   || event == EVENT_MODECHANGED)
	    autocmd_fname = NULL;
	else if (fname != NULL && !ends_excmd(*fname))
	    autocmd_fname = fname;
	else if (buf != NULL)
	    autocmd_fname = buf->b_ffname;
	else
	    autocmd_fname = NULL;
    }
    else
	autocmd_fname = fname_io;
    if (autocmd_fname != NULL)
	autocmd_fname = vim_strsave(autocmd_fname);
    autocmd_fname_full = FALSE;  
    if (buf == NULL)
	autocmd_bufnr = 0;
    else
	autocmd_bufnr = buf->b_fnum;
    if (fname == NULL || *fname == NUL)
    {
	if (buf == NULL)
	    fname = NULL;
	else
	{
#ifdef FEAT_SYN_HL
	    if (event == EVENT_SYNTAX)
		fname = buf->b_p_syn;
	    else
#endif
		if (event == EVENT_FILETYPE)
		    fname = buf->b_p_ft;
		else
		{
		    if (buf->b_sfname != NULL)
			sfname = vim_strsave(buf->b_sfname);
		    fname = buf->b_ffname;
		}
	}
	if (fname == NULL)
	    fname = (char_u *)"";
	fname = vim_strsave(fname);	 
    }
    else
    {
	sfname = vim_strsave(fname);
	if (event == EVENT_FILETYPE
		|| event == EVENT_SYNTAX
		|| event == EVENT_CMDLINECHANGED
		|| event == EVENT_CMDLINEENTER
		|| event == EVENT_CMDLINELEAVE
		|| event == EVENT_CMDWINENTER
		|| event == EVENT_CMDWINLEAVE
		|| event == EVENT_CMDUNDEFINED
		|| event == EVENT_FUNCUNDEFINED
		|| event == EVENT_REMOTEREPLY
		|| event == EVENT_SPELLFILEMISSING
		|| event == EVENT_QUICKFIXCMDPRE
		|| event == EVENT_COLORSCHEME
		|| event == EVENT_COLORSCHEMEPRE
		|| event == EVENT_OPTIONSET
		|| event == EVENT_QUICKFIXCMDPOST
		|| event == EVENT_DIRCHANGED
		|| event == EVENT_DIRCHANGEDPRE
		|| event == EVENT_MODECHANGED
		|| event == EVENT_USER
		|| event == EVENT_WINCLOSED
		|| event == EVENT_WINSCROLLED)
	{
	    fname = vim_strsave(fname);
	    autocmd_fname_full = TRUE;  
	}
	else
	    fname = FullName_save(fname, FALSE);
    }
    if (fname == NULL)	     
    {
	vim_free(sfname);
	retval = FALSE;
	goto BYPASS_AU;
    }
#ifdef BACKSLASH_IN_FILENAME
    if (sfname != NULL)
	forward_slash(sfname);
    forward_slash(fname);
#endif
#ifdef VMS
    if (sfname != NULL)
	vms_remove_version(sfname);
    vms_remove_version(fname);
#endif
    autocmd_match = fname;
    ++RedrawingDisabled;
    estack_push(ETYPE_AUCMD, NULL, 0);
    ESTACK_CHECK_SETUP
    save_current_sctx = current_sctx;
#ifdef FEAT_EVAL
# ifdef FEAT_PROFILE
    if (do_profiling == PROF_YES)
	prof_child_enter(&wait_time);  
# endif
    save_funccal(&funccal_entry);
#endif
    if (!autocmd_busy)
    {
	save_search_patterns();
	if (!ins_compl_active())
	{
	    saveRedobuff(&save_redo);
	    did_save_redobuff = TRUE;
	}
	did_filetype = keep_filetype;
    }
    autocmd_busy = TRUE;
    filechangeshell_busy = (event == EVENT_FILECHANGEDSHELL);
    ++nesting;		 
    if (event == EVENT_FILETYPE)
	did_filetype = TRUE;
    tail = gettail(fname);
    CLEAR_FIELD(patcmd);
    patcmd.curpat = first_autopat[(int)event];
    patcmd.group = group;
    patcmd.fname = fname;
    patcmd.sfname = sfname;
    patcmd.tail = tail;
    patcmd.event = event;
    patcmd.arg_bufnr = autocmd_bufnr;
    auto_next_pat(&patcmd, FALSE);
    if (patcmd.curpat != NULL)
    {
	patcmd.next = active_apc_list;
	active_apc_list = &patcmd;
#ifdef FEAT_EVAL
	save_cmdbang = (long)get_vim_var_nr(VV_CMDBANG);
	if (eap != NULL)
	{
	    save_cmdarg = set_cmdarg(eap, NULL);
	    set_vim_var_nr(VV_CMDBANG, (long)eap->forceit);
	}
	else
	    save_cmdarg = NULL;	 
#endif
	retval = TRUE;
	for (ap = patcmd.curpat; ap->next != NULL; ap = ap->next)
	    ap->last = FALSE;
	ap->last = TRUE;
	if (nesting == 1)
	    check_lnums(TRUE);
	save_did_emsg = did_emsg;
	do_cmdline(NULL, getnextac, (void *)&patcmd,
				     DOCMD_NOWAIT|DOCMD_VERBOSE|DOCMD_REPEAT);
	did_emsg += save_did_emsg;
	if (nesting == 1)
	    reset_lnums();
#ifdef FEAT_EVAL
	if (eap != NULL)
	{
	    (void)set_cmdarg(NULL, save_cmdarg);
	    set_vim_var_nr(VV_CMDBANG, save_cmdbang);
	}
#endif
	if (active_apc_list == &patcmd)	     
	    active_apc_list = patcmd.next;
    }
    --RedrawingDisabled;
    autocmd_busy = save_autocmd_busy;
    filechangeshell_busy = FALSE;
    autocmd_nested = save_autocmd_nested;
    vim_free(SOURCING_NAME);
    ESTACK_CHECK_NOW
    estack_pop();
    vim_free(autocmd_fname);
    autocmd_fname = save_autocmd_fname;
    autocmd_fname_full = save_autocmd_fname_full;
    autocmd_bufnr = save_autocmd_bufnr;
    autocmd_match = save_autocmd_match;
    current_sctx = save_current_sctx;
#ifdef FEAT_EVAL
    restore_funccal();
# ifdef FEAT_PROFILE
    if (do_profiling == PROF_YES)
	prof_child_exit(&wait_time);
# endif
#endif
    KeyTyped = save_KeyTyped;
    vim_free(fname);
    vim_free(sfname);
    --nesting;		 
    if (!autocmd_busy)
    {
	restore_search_patterns();
	if (did_save_redobuff)
	    restoreRedobuff(&save_redo);
	did_filetype = FALSE;
	while (au_pending_free_buf != NULL)
	{
	    buf_T *b = au_pending_free_buf->b_next;
	    vim_free(au_pending_free_buf);
	    au_pending_free_buf = b;
	}
	while (au_pending_free_win != NULL)
	{
	    win_T *w = au_pending_free_win->w_next;
	    vim_free(au_pending_free_win);
	    au_pending_free_win = w;
	}
    }
    if (curbuf == old_curbuf
	    && (event == EVENT_BUFREADPOST
		|| event == EVENT_BUFWRITEPOST
		|| event == EVENT_FILEAPPENDPOST
		|| event == EVENT_VIMLEAVE
		|| event == EVENT_VIMLEAVEPRE))
    {
	if (curbuf->b_changed != save_changed)
	    need_maketitle = TRUE;
	curbuf->b_changed = save_changed;
    }
    au_cleanup();	 
BYPASS_AU:
    if (event == EVENT_BUFWIPEOUT && buf != NULL)
	aubuflocal_remove(buf);
    if (retval == OK && event == EVENT_FILETYPE)
	au_did_filetype = TRUE;
    return retval;
}