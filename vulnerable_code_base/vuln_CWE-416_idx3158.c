do_ecmd(
    int		fnum,
    char_u	*ffname,
    char_u	*sfname,
    exarg_T	*eap,			 
    linenr_T	newlnum,
    int		flags,
    win_T	*oldwin)
{
    int		other_file;		 
    int		oldbuf;			 
    int		auto_buf = FALSE;	 
    char_u	*new_name = NULL;
#if defined(FEAT_EVAL)
    int		did_set_swapcommand = FALSE;
#endif
    buf_T	*buf;
    bufref_T	bufref;
    bufref_T	old_curbuf;
    char_u	*free_fname = NULL;
#ifdef FEAT_BROWSE
    char_u	dot_path[] = ".";
    char_u	*browse_file = NULL;
#endif
    int		retval = FAIL;
    long	n;
    pos_T	orig_pos;
    linenr_T	topline = 0;
    int		newcol = -1;
    int		solcol = -1;
    pos_T	*pos;
    char_u	*command = NULL;
#ifdef FEAT_SPELL
    int		did_get_winopts = FALSE;
#endif
    int		readfile_flags = 0;
    int		did_inc_redrawing_disabled = FALSE;
    long	*so_ptr = curwin->w_p_so >= 0 ? &curwin->w_p_so : &p_so;
#ifdef FEAT_PROP_POPUP
    if (ERROR_IF_TERM_POPUP_WINDOW)
	return FAIL;
#endif
    if (eap != NULL)
	command = eap->do_ecmd_cmd;
    set_bufref(&old_curbuf, curbuf);
    if (fnum != 0)
    {
	if (fnum == curbuf->b_fnum)	 
	    return OK;			 
	other_file = TRUE;
    }
    else
    {
#ifdef FEAT_BROWSE
	if ((cmdmod.cmod_flags & CMOD_BROWSE) && !exiting)
	{
	    if (
# ifdef FEAT_GUI
		!gui.in_use &&
# endif
		    au_has_group((char_u *)"FileExplorer"))
	    {
		if (ffname == NULL || !mch_isdir(ffname))
		    ffname = dot_path;
	    }
	    else
	    {
		browse_file = do_browse(0, (char_u *)_("Edit File"), ffname,
						    NULL, NULL, NULL, curbuf);
		if (browse_file == NULL)
		    goto theend;
		ffname = browse_file;
	    }
	}
#endif
	if (sfname == NULL)
	    sfname = ffname;
#ifdef USE_FNAME_CASE
	if (sfname != NULL)
	    fname_case(sfname, 0);    
#endif
	if ((flags & (ECMD_ADDBUF | ECMD_ALTBUF))
					 && (ffname == NULL || *ffname == NUL))
	    goto theend;
	if (ffname == NULL)
	    other_file = TRUE;
	else if (*ffname == NUL && curbuf->b_ffname == NULL)
	    other_file = FALSE;
	else
	{
	    if (*ffname == NUL)		     
	    {
		ffname = curbuf->b_ffname;
		sfname = curbuf->b_fname;
	    }
	    free_fname = fix_fname(ffname);  
	    if (free_fname != NULL)
		ffname = free_fname;
	    other_file = otherfile(ffname);
	}
    }
    if (  ((!other_file && !(flags & ECMD_OLDBUF))
	    || (curbuf->b_nwindows == 1
		&& !(flags & (ECMD_HIDE | ECMD_ADDBUF | ECMD_ALTBUF))))
	&& check_changed(curbuf, (p_awa ? CCGD_AW : 0)
			       | (other_file ? 0 : CCGD_MULTWIN)
			       | ((flags & ECMD_FORCEIT) ? CCGD_FORCEIT : 0)
			       | (eap == NULL ? 0 : CCGD_EXCMD)))
    {
	if (fnum == 0 && other_file && ffname != NULL)
	    (void)setaltfname(ffname, sfname, newlnum < 0 ? 0 : newlnum);
	goto theend;
    }
    reset_VIsual();
#if defined(FEAT_EVAL)
    if ((command != NULL || newlnum > (linenr_T)0)
	    && *get_vim_var_str(VV_SWAPCOMMAND) == NUL)
    {
	int	len;
	char_u	*p;
	if (command != NULL)
	    len = (int)STRLEN(command) + 3;
	else
	    len = 30;
	p = alloc(len);
	if (p != NULL)
	{
	    if (command != NULL)
		vim_snprintf((char *)p, len, ":%s\r", command);
	    else
		vim_snprintf((char *)p, len, "%ldG", (long)newlnum);
	    set_vim_var_string(VV_SWAPCOMMAND, p, -1);
	    did_set_swapcommand = TRUE;
	    vim_free(p);
	}
    }
#endif
    if (other_file)
    {
	int prev_alt_fnum = curwin->w_alt_fnum;
	if (!(flags & (ECMD_ADDBUF | ECMD_ALTBUF)))
	{
	    if ((cmdmod.cmod_flags & CMOD_KEEPALT) == 0)
		curwin->w_alt_fnum = curbuf->b_fnum;
	    if (oldwin != NULL)
		buflist_altfpos(oldwin);
	}
	if (fnum)
	    buf = buflist_findnr(fnum);
	else
	{
	    if (flags & (ECMD_ADDBUF | ECMD_ALTBUF))
	    {
		linenr_T	tlnum = 0;
		buf_T		*newbuf;
		if (command != NULL)
		{
		    tlnum = atol((char *)command);
		    if (tlnum <= 0)
			tlnum = 1L;
		}
		newbuf = buflist_new(ffname, sfname, tlnum,
						    BLN_LISTED | BLN_NOCURWIN);
		if (newbuf != NULL)
		{
		    if (flags & ECMD_ALTBUF)
			curwin->w_alt_fnum = newbuf->b_fnum;
		    if (tlnum > 0)
			newbuf->b_last_cursor.lnum = tlnum;
		}
		goto theend;
	    }
	    buf = buflist_new(ffname, sfname, 0L,
		    BLN_CURBUF | ((flags & ECMD_SET_HELP) ? 0 : BLN_LISTED));
	    if (oldwin != NULL)
		oldwin = curwin;
	    set_bufref(&old_curbuf, curbuf);
	}
	if (buf == NULL)
	    goto theend;
	if (curwin->w_alt_fnum == buf->b_fnum && prev_alt_fnum != 0)
	    curwin->w_alt_fnum = prev_alt_fnum;
	if (buf->b_ml.ml_mfp == NULL)		 
	{
	    oldbuf = FALSE;
	}
	else					 
	{
	    oldbuf = TRUE;
	    set_bufref(&bufref, buf);
	    (void)buf_check_timestamp(buf, FALSE);
	    if (!bufref_valid(&bufref) || curbuf != old_curbuf.br_buf)
		goto theend;
#ifdef FEAT_EVAL
	    if (aborting())	     
		goto theend;
#endif
	}
	if ((oldbuf && newlnum == ECMD_LASTL) || newlnum == ECMD_LAST)
	{
	    pos = buflist_findfpos(buf);
	    newlnum = pos->lnum;
	    solcol = pos->col;
	}
	if (buf != curbuf)
	{
	    bufref_T	save_au_new_curbuf;
	    int		save_cmdwin_type = cmdwin_type;
	    cmdwin_type = 0;
	    if (buf->b_fname != NULL)
		new_name = vim_strsave(buf->b_fname);
	    save_au_new_curbuf = au_new_curbuf;
	    set_bufref(&au_new_curbuf, buf);
	    apply_autocmds(EVENT_BUFLEAVE, NULL, NULL, FALSE, curbuf);
	    cmdwin_type = save_cmdwin_type;
	    if (!bufref_valid(&au_new_curbuf))
	    {
		delbuf_msg(new_name);	 
		au_new_curbuf = save_au_new_curbuf;
		goto theend;
	    }
#ifdef FEAT_EVAL
	    if (aborting())	     
	    {
		vim_free(new_name);
		au_new_curbuf = save_au_new_curbuf;
		goto theend;
	    }
#endif
	    if (buf == curbuf)		 
		auto_buf = TRUE;
	    else
	    {
		win_T	    *the_curwin = curwin;
		int	    did_decrement;
		buf_T	    *was_curbuf = curbuf;
		the_curwin->w_closing = TRUE;
		++buf->b_locked;
		if (curbuf == old_curbuf.br_buf)
		    buf_copy_options(buf, BCO_ENTER);
		u_sync(FALSE);
		did_decrement = close_buffer(oldwin, curbuf,
			 (flags & ECMD_HIDE) ? 0 : DOBUF_UNLOAD, FALSE, FALSE);
		if (win_valid(the_curwin))
		    the_curwin->w_closing = FALSE;
		--buf->b_locked;
#ifdef FEAT_EVAL
		if (aborting() && curwin->w_buffer != NULL)
		{
		    vim_free(new_name);
		    au_new_curbuf = save_au_new_curbuf;
		    goto theend;
		}
#endif
		if (!bufref_valid(&au_new_curbuf))
		{
		    delbuf_msg(new_name);	 
		    au_new_curbuf = save_au_new_curbuf;
		    goto theend;
		}
		if (buf == curbuf)		 
		{
		    if (did_decrement && buf_valid(was_curbuf))
			++was_curbuf->b_nwindows;
		    if (win_valid_any_tab(oldwin) && oldwin->w_buffer == NULL)
			oldwin->w_buffer = was_curbuf;
		    auto_buf = TRUE;
		}
		else
		{
#ifdef FEAT_SYN_HL
		    if (curwin->w_buffer == NULL
			    || curwin->w_s == &(curwin->w_buffer->b_s))
			curwin->w_s = &(buf->b_s);
#endif
		    curwin->w_buffer = buf;
		    curbuf = buf;
		    ++curbuf->b_nwindows;
		    if (!oldbuf && eap != NULL)
		    {
			set_file_options(TRUE, eap);
			set_forced_fenc(eap);
		    }
		}
		get_winopts(curbuf);
#ifdef FEAT_SPELL
		did_get_winopts = TRUE;
#endif
	    }
	    vim_free(new_name);
	    au_new_curbuf = save_au_new_curbuf;
	}
	curwin->w_pcmark.lnum = 1;
	curwin->w_pcmark.col = 0;
    }
    else  
    {
	if ((flags & (ECMD_ADDBUF | ECMD_ALTBUF)) || check_fname() == FAIL)
	    goto theend;
	oldbuf = (flags & ECMD_OLDBUF);
    }
    ++RedrawingDisabled;
    did_inc_redrawing_disabled = TRUE;
    buf = curbuf;
    if ((flags & ECMD_SET_HELP) || keep_help_flag)
    {
	prepare_help_buffer();
    }
    else
    {
	if (!curbuf->b_help)
	    set_buflisted(TRUE);
    }
    if (buf != curbuf)
	goto theend;
#ifdef FEAT_EVAL
    if (aborting())	     
	goto theend;
#endif
    did_filetype = FALSE;
    if (!other_file && !oldbuf)		 
    {
	set_last_cursor(curwin);	 
	if (newlnum == ECMD_LAST || newlnum == ECMD_LASTL)
	{
	    newlnum = curwin->w_cursor.lnum;
	    solcol = curwin->w_cursor.col;
	}
	buf = curbuf;
	if (buf->b_fname != NULL)
	    new_name = vim_strsave(buf->b_fname);
	else
	    new_name = NULL;
	set_bufref(&bufref, buf);
	if (!(curbuf->b_flags & BF_NEVERLOADED)
		&& (p_ur < 0 || curbuf->b_ml.ml_line_count <= p_ur))
	{
	    u_sync(FALSE);
	    if (u_savecommon(0, curbuf->b_ml.ml_line_count + 1, 0, TRUE)
								     == FAIL)
	    {
		vim_free(new_name);
		goto theend;
	    }
	    u_unchanged(curbuf);
	    buf_freeall(curbuf, BFA_KEEP_UNDO);
	    readfile_flags = READ_KEEP_UNDO;
	}
	else
	    buf_freeall(curbuf, 0);    
	if (!bufref_valid(&bufref))
	{
	    delbuf_msg(new_name);	 
	    goto theend;
	}
	vim_free(new_name);
	if (buf != curbuf)
	    goto theend;
#ifdef FEAT_EVAL
	if (aborting())	     
	    goto theend;
#endif
	buf_clear_file(curbuf);
	curbuf->b_op_start.lnum = 0;	 
	curbuf->b_op_end.lnum = 0;
    }
    retval = OK;
    if (!other_file)
	curbuf->b_flags &= ~BF_NOTEDITED;
    check_arg_idx(curwin);
    if (!auto_buf)
    {
	curwin_init();
#ifdef FEAT_FOLDING
	{
	    win_T	    *win;
	    tabpage_T	    *tp;
	    FOR_ALL_TAB_WINDOWS(tp, win)
		if (win->w_buffer == curbuf)
		    foldUpdateAll(win);
	}
#endif
	DO_AUTOCHDIR;
	orig_pos = curwin->w_cursor;
	topline = curwin->w_topline;
	if (!oldbuf)			     
	{
#ifdef FEAT_PROP_POPUP
	    if (WIN_IS_POPUP(curwin))
		curbuf->b_flags |= BF_NO_SEA;
#endif
	    swap_exists_action = SEA_DIALOG;
	    curbuf->b_flags |= BF_CHECK_RO;  
	    if (flags & ECMD_NOWINENTER)
		readfile_flags |= READ_NOWINENTER;
#if defined(FEAT_EVAL)
	    if (should_abort(open_buffer(FALSE, eap, readfile_flags)))
		retval = FAIL;
#else
	    (void)open_buffer(FALSE, eap, readfile_flags);
#endif
#ifdef FEAT_PROP_POPUP
	    curbuf->b_flags &= ~BF_NO_SEA;
#endif
	    if (swap_exists_action == SEA_QUIT)
		retval = FAIL;
	    handle_swap_exists(&old_curbuf);
	}
	else
	{
	    do_modelines(OPT_WINONLY);
	    apply_autocmds_retval(EVENT_BUFENTER, NULL, NULL, FALSE,
							      curbuf, &retval);
	    if ((flags & ECMD_NOWINENTER) == 0)
		apply_autocmds_retval(EVENT_BUFWINENTER, NULL, NULL, FALSE,
							      curbuf, &retval);
	}
	check_arg_idx(curwin);
	if (!EQUAL_POS(curwin->w_cursor, orig_pos))
	{
	    char_u *text = ml_get_curline();
	    if (curwin->w_cursor.lnum != orig_pos.lnum
		    || curwin->w_cursor.col != (int)(skipwhite(text) - text))
	    {
		newlnum = curwin->w_cursor.lnum;
		newcol = curwin->w_cursor.col;
	    }
	}
	if (curwin->w_topline == topline)
	    topline = 0;
	changed_line_abv_curs();
	maketitle();
#if defined(FEAT_PROP_POPUP) && defined(FEAT_QUICKFIX)
	if (WIN_IS_POPUP(curwin) && curwin->w_p_pvw && retval != FAIL)
	    popup_set_title(curwin);
#endif
    }
#ifdef FEAT_DIFF
    if (curwin->w_p_diff)
    {
	diff_buf_add(curbuf);
	diff_invalidate(curbuf);
    }
#endif
#ifdef FEAT_SPELL
    if (did_get_winopts && curwin->w_p_spell && *curwin->w_s->b_p_spl != NUL)
	(void)parse_spelllang(curwin);
#endif
    if (command == NULL)
    {
	if (newcol >= 0)	 
	{
	    curwin->w_cursor.lnum = newlnum;
	    curwin->w_cursor.col = newcol;
	    check_cursor();
	}
	else if (newlnum > 0)	 
	{
	    curwin->w_cursor.lnum = newlnum;
	    check_cursor_lnum();
	    if (solcol >= 0 && !p_sol)
	    {
		curwin->w_cursor.col = solcol;
		check_cursor_col();
		curwin->w_cursor.coladd = 0;
		curwin->w_set_curswant = TRUE;
	    }
	    else
		beginline(BL_SOL | BL_FIX);
	}
	else			 
	{
	    if (exmode_active)
		curwin->w_cursor.lnum = curbuf->b_ml.ml_line_count;
	    beginline(BL_WHITE | BL_FIX);
	}
    }
    check_lnums(FALSE);
    if (oldbuf && !auto_buf)
    {
	int	msg_scroll_save = msg_scroll;
	if (shortmess(SHM_OVERALL) && !exiting && p_verbose == 0)
	    msg_scroll = FALSE;
	if (!msg_scroll)	 
	    check_for_delay(FALSE);
	msg_start();
	msg_scroll = msg_scroll_save;
	msg_scrolled_ign = TRUE;
	if (!shortmess(SHM_FILEINFO))
	    fileinfo(FALSE, TRUE, FALSE);
	msg_scrolled_ign = FALSE;
    }
#ifdef FEAT_VIMINFO
    curbuf->b_last_used = vim_time();
#endif
    if (command != NULL)
	do_cmdline(command, NULL, NULL, DOCMD_VERBOSE|DOCMD_RANGEOK);
#ifdef FEAT_KEYMAP
    if (curbuf->b_kmap_state & KEYMAP_INIT)
	(void)keymap_init();
#endif
    if (RedrawingDisabled > 0)
	--RedrawingDisabled;
    did_inc_redrawing_disabled = FALSE;
    if (!skip_redraw)
    {
	n = *so_ptr;
	if (topline == 0 && command == NULL)
	    *so_ptr = 9999;		 
	update_topline();
	curwin->w_scbind_pos = curwin->w_topline;
	*so_ptr = n;
	redraw_curbuf_later(UPD_NOT_VALID);	 
    }
    if (p_im && (State & MODE_INSERT) == 0)
	need_start_insertmode = TRUE;
#ifdef FEAT_AUTOCHDIR
    if (p_acd && curbuf->b_ffname != NULL)
    {
	char_u	curdir[MAXPATHL];
	char_u	filedir[MAXPATHL];
	vim_strncpy(filedir, curbuf->b_ffname, MAXPATHL - 1);
	*gettail_sep(filedir) = NUL;
	if (mch_dirname(curdir, MAXPATHL) != FAIL
		&& vim_fnamecmp(curdir, filedir) != 0)
	    do_autochdir();
    }
#endif
#if defined(FEAT_NETBEANS_INTG)
    if (curbuf->b_ffname != NULL)
    {
# ifdef FEAT_NETBEANS_INTG
	if ((flags & ECMD_SET_HELP) != ECMD_SET_HELP)
	    netbeans_file_opened(curbuf);
# endif
    }
#endif
theend:
    if (did_inc_redrawing_disabled && RedrawingDisabled > 0)
	--RedrawingDisabled;
#if defined(FEAT_EVAL)
    if (did_set_swapcommand)
	set_vim_var_string(VV_SWAPCOMMAND, NULL, -1);
#endif
#ifdef FEAT_BROWSE
    vim_free(browse_file);
#endif
    vim_free(free_fname);
    return retval;
}