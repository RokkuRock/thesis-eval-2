readfile(
    char_u	*fname,
    char_u	*sfname,
    linenr_T	from,
    linenr_T	lines_to_skip,
    linenr_T	lines_to_read,
    exarg_T	*eap,			 
    int		flags)
{
    int		fd = 0;
    int		newfile = (flags & READ_NEW);
    int		check_readonly;
    int		filtering = (flags & READ_FILTER);
    int		read_stdin = (flags & READ_STDIN);
    int		read_buffer = (flags & READ_BUFFER);
    int		read_fifo = (flags & READ_FIFO);
    int		set_options = newfile || read_buffer
					   || (eap != NULL && eap->read_edit);
    linenr_T	read_buf_lnum = 1;	 
    colnr_T	read_buf_col = 0;	 
    char_u	c;
    linenr_T	lnum = from;
    char_u	*ptr = NULL;		 
    char_u	*buffer = NULL;		 
    char_u	*new_buffer = NULL;	 
    char_u	*line_start = NULL;	 
    int		wasempty;		 
    colnr_T	len;
    long	size = 0;
    char_u	*p;
    off_T	filesize = 0;
    int		skip_read = FALSE;
#ifdef FEAT_CRYPT
    char_u	*cryptkey = NULL;
    int		did_ask_for_key = FALSE;
#endif
#ifdef FEAT_PERSISTENT_UNDO
    context_sha256_T sha_ctx;
    int		read_undo_file = FALSE;
#endif
    int		split = 0;		 
#define UNKNOWN	 0x0fffffff		 
    linenr_T	linecnt;
    int		error = FALSE;		 
    int		ff_error = EOL_UNKNOWN;  
    long	linerest = 0;		 
#ifdef UNIX
    int		perm = 0;
    int		swap_mode = -1;		 
#else
    int		perm;
#endif
    int		fileformat = 0;		 
    int		keep_fileformat = FALSE;
    stat_T	st;
    int		file_readonly;
    linenr_T	skip_count = 0;
    linenr_T	read_count = 0;
    int		msg_save = msg_scroll;
    linenr_T	read_no_eol_lnum = 0;    
    int		try_mac;
    int		try_dos;
    int		try_unix;
    int		file_rewind = FALSE;
#ifdef FEAT_MBYTE
    int		can_retry;
    linenr_T	conv_error = 0;		 
    linenr_T	illegal_byte = 0;	 
    int		keep_dest_enc = FALSE;	 
    int		bad_char_behavior = BAD_REPLACE;
    char_u	*tmpname = NULL;	 
    int		fio_flags = 0;
    char_u	*fenc;			 
    int		fenc_alloced;		 
    char_u	*fenc_next = NULL;	 
    int		advance_fenc = FALSE;
    long	real_size = 0;
# ifdef USE_ICONV
    iconv_t	iconv_fd = (iconv_t)-1;	 
#  ifdef FEAT_EVAL
    int		did_iconv = FALSE;	 
#  endif
# endif
    int		converted = FALSE;	 
    int		notconverted = FALSE;	 
    char_u	conv_rest[CONV_RESTLEN];
    int		conv_restlen = 0;	 
#endif
#ifdef FEAT_AUTOCMD
    buf_T	*old_curbuf;
    char_u	*old_b_ffname;
    char_u	*old_b_fname;
    int		using_b_ffname;
    int		using_b_fname;
#endif
#ifdef FEAT_AUTOCMD
    au_did_filetype = FALSE;  
#endif
    curbuf->b_no_eol_lnum = 0;	 
    if (curbuf->b_ffname == NULL
	    && !filtering
	    && fname != NULL
	    && vim_strchr(p_cpo, CPO_FNAMER) != NULL
	    && !(flags & READ_DUMMY))
    {
	if (set_rw_fname(fname, sfname) == FAIL)
	    return FAIL;
    }
#ifdef FEAT_AUTOCMD
    old_curbuf = curbuf;
    old_b_ffname = curbuf->b_ffname;
    old_b_fname = curbuf->b_fname;
    using_b_ffname = (fname == curbuf->b_ffname)
					      || (sfname == curbuf->b_ffname);
    using_b_fname = (fname == curbuf->b_fname) || (sfname == curbuf->b_fname);
#endif
    ex_no_reprint = TRUE;
    need_fileinfo = FALSE;
    if (sfname == NULL)
	sfname = fname;
#if defined(UNIX)
    fname = sfname;
#endif
#ifdef FEAT_AUTOCMD
    if (!filtering && !read_stdin && !read_buffer)
    {
	pos_T	    pos;
	pos = curbuf->b_op_start;
	curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
	curbuf->b_op_start.col = 0;
	if (newfile)
	{
	    if (apply_autocmds_exarg(EVENT_BUFREADCMD, NULL, sfname,
							  FALSE, curbuf, eap))
#ifdef FEAT_EVAL
		return aborting() ? FAIL : OK;
#else
		return OK;
#endif
	}
	else if (apply_autocmds_exarg(EVENT_FILEREADCMD, sfname, sfname,
							    FALSE, NULL, eap))
#ifdef FEAT_EVAL
	    return aborting() ? FAIL : OK;
#else
	    return OK;
#endif
	curbuf->b_op_start = pos;
    }
#endif
    if ((shortmess(SHM_OVER) || curbuf->b_help) && p_verbose == 0)
	msg_scroll = FALSE;	 
    else
	msg_scroll = TRUE;	 
    if (fname != NULL && *fname != NUL)
    {
	p = fname + STRLEN(fname);
	if (after_pathsep(fname, p) || STRLEN(fname) >= MAXPATHL)
	{
	    filemess(curbuf, fname, (char_u *)_("Illegal file name"), 0);
	    msg_end();
	    msg_scroll = msg_save;
	    return FAIL;
	}
    }
    if (!read_stdin && !read_buffer && !read_fifo)
    {
#ifdef UNIX
	perm = mch_getperm(fname);
	if (perm >= 0 && !S_ISREG(perm)		     
# ifdef S_ISFIFO
		      && !S_ISFIFO(perm)	     
# endif
# ifdef S_ISSOCK
		      && !S_ISSOCK(perm)	     
# endif
# ifdef OPEN_CHR_FILES
		      && !(S_ISCHR(perm) && is_dev_fd_file(fname))
# endif
						)
	{
	    int retval = FAIL;
	    if (S_ISDIR(perm))
	    {
		filemess(curbuf, fname, (char_u *)_("is a directory"), 0);
		retval = NOTDONE;
	    }
	    else
		filemess(curbuf, fname, (char_u *)_("is not a file"), 0);
	    msg_end();
	    msg_scroll = msg_save;
	    return retval;
	}
#endif
#if defined(MSWIN)
	if (!p_odev && mch_nodetype(fname) == NODE_WRITABLE)
	{
	    filemess(curbuf, fname, (char_u *)_("is a device (disabled with 'opendevice' option)"), 0);
	    msg_end();
	    msg_scroll = msg_save;
	    return FAIL;
	}
#endif
    }
    set_file_options(set_options, eap);
    check_readonly = (newfile && (curbuf->b_flags & BF_CHECK_RO));
    if (check_readonly && !readonlymode)
	curbuf->b_p_ro = FALSE;
    if (newfile && !read_stdin && !read_buffer && !read_fifo)
    {
	if (mch_stat((char *)fname, &st) >= 0)
	{
	    buf_store_time(curbuf, &st, fname);
	    curbuf->b_mtime_read = curbuf->b_mtime;
#ifdef UNIX
	    swap_mode = (st.st_mode & 0644) | 0600;
#endif
#ifdef FEAT_CW_EDITOR
	    (void)GetFSSpecFromPath(curbuf->b_ffname, &curbuf->b_FSSpec);
#endif
#ifdef VMS
	    curbuf->b_fab_rfm = st.st_fab_rfm;
	    curbuf->b_fab_rat = st.st_fab_rat;
	    curbuf->b_fab_mrs = st.st_fab_mrs;
#endif
	}
	else
	{
	    curbuf->b_mtime = 0;
	    curbuf->b_mtime_read = 0;
	    curbuf->b_orig_size = 0;
	    curbuf->b_orig_mode = 0;
	}
	curbuf->b_flags &= ~(BF_NEW | BF_NEW_W);
    }
    file_readonly = FALSE;
    if (read_stdin)
    {
#if defined(MSWIN)
	setmode(0, O_BINARY);
#endif
    }
    else if (!read_buffer)
    {
#ifdef USE_MCH_ACCESS
	if (
# ifdef UNIX
	    !(perm & 0222) ||
# endif
				mch_access((char *)fname, W_OK))
	    file_readonly = TRUE;
	fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0);
#else
	if (!newfile
		|| readonlymode
		|| (fd = mch_open((char *)fname, O_RDWR | O_EXTRA, 0)) < 0)
	{
	    file_readonly = TRUE;
	    fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0);
	}
#endif
    }
    if (fd < 0)			     
    {
#ifndef UNIX
	int	isdir_f;
#endif
	msg_scroll = msg_save;
#ifndef UNIX
	isdir_f = (mch_isdir(fname));
	perm = mch_getperm(fname);   
	if (isdir_f)
	{
	    filemess(curbuf, sfname, (char_u *)_("is a directory"), 0);
	    curbuf->b_p_ro = TRUE;	 
	}
	else
#endif
	    if (newfile)
	    {
		if (perm < 0
#ifdef ENOENT
			&& errno == ENOENT
#endif
		   )
		{
		    curbuf->b_flags |= BF_NEW;
#ifdef FEAT_QUICKFIX
		    if (!bt_dontwrite(curbuf))
#endif
		    {
			check_need_swap(newfile);
#ifdef FEAT_AUTOCMD
			if (curbuf != old_curbuf
				|| (using_b_ffname
					&& (old_b_ffname != curbuf->b_ffname))
				|| (using_b_fname
					 && (old_b_fname != curbuf->b_fname)))
			{
			    EMSG(_(e_auchangedbuf));
			    return FAIL;
			}
#endif
		    }
		    if (dir_of_file_exists(fname))
			filemess(curbuf, sfname, (char_u *)_("[New File]"), 0);
		    else
			filemess(curbuf, sfname,
					   (char_u *)_("[New DIRECTORY]"), 0);
#ifdef FEAT_VIMINFO
		    check_marks_read();
#endif
#ifdef FEAT_MBYTE
		    if (eap != NULL)
			set_forced_fenc(eap);
#endif
#ifdef FEAT_AUTOCMD
		    apply_autocmds_exarg(EVENT_BUFNEWFILE, sfname, sfname,
							  FALSE, curbuf, eap);
#endif
		    save_file_ff(curbuf);
#if defined(FEAT_AUTOCMD) && defined(FEAT_EVAL)
		    if (aborting())    
			return FAIL;
#endif
		    return OK;	     
		}
		else
		{
		    filemess(curbuf, sfname, (char_u *)(
# ifdef EFBIG
			    (errno == EFBIG) ? _("[File too big]") :
# endif
# ifdef EOVERFLOW
			    (errno == EOVERFLOW) ? _("[File too big]") :
# endif
						_("[Permission Denied]")), 0);
		    curbuf->b_p_ro = TRUE;	 
		}
	    }
	return FAIL;
    }
    if ((check_readonly && file_readonly) || curbuf->b_help)
	curbuf->b_p_ro = TRUE;
    if (set_options)
    {
	if (!read_buffer)
	{
	    curbuf->b_p_eol = TRUE;
	    curbuf->b_start_eol = TRUE;
	}
#ifdef FEAT_MBYTE
	curbuf->b_p_bomb = FALSE;
	curbuf->b_start_bomb = FALSE;
#endif
    }
#ifdef FEAT_QUICKFIX
    if (!bt_dontwrite(curbuf))
#endif
    {
	check_need_swap(newfile);
#ifdef FEAT_AUTOCMD
	if (!read_stdin && (curbuf != old_curbuf
		|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
		|| (using_b_fname && (old_b_fname != curbuf->b_fname))))
	{
	    EMSG(_(e_auchangedbuf));
	    if (!read_buffer)
		close(fd);
	    return FAIL;
	}
#endif
#ifdef UNIX
	if (swap_mode > 0 && curbuf->b_ml.ml_mfp != NULL
			  && curbuf->b_ml.ml_mfp->mf_fname != NULL)
	    (void)mch_setperm(curbuf->b_ml.ml_mfp->mf_fname, (long)swap_mode);
#endif
    }
#if defined(HAS_SWAP_EXISTS_ACTION)
    if (swap_exists_action == SEA_QUIT)
    {
	if (!read_buffer && !read_stdin)
	    close(fd);
	return FAIL;
    }
#endif
    ++no_wait_return;	     
    curbuf->b_op_start.lnum = ((from == 0) ? 1 : from);
    curbuf->b_op_start.col = 0;
    try_mac = (vim_strchr(p_ffs, 'm') != NULL);
    try_dos = (vim_strchr(p_ffs, 'd') != NULL);
    try_unix = (vim_strchr(p_ffs, 'x') != NULL);
#ifdef FEAT_AUTOCMD
    if (!read_buffer)
    {
	int	m = msg_scroll;
	int	n = msg_scrolled;
	if (!read_stdin)
	    close(fd);		 
	msg_scroll = TRUE;
	if (filtering)
	    apply_autocmds_exarg(EVENT_FILTERREADPRE, NULL, sfname,
							  FALSE, curbuf, eap);
	else if (read_stdin)
	    apply_autocmds_exarg(EVENT_STDINREADPRE, NULL, sfname,
							  FALSE, curbuf, eap);
	else if (newfile)
	    apply_autocmds_exarg(EVENT_BUFREADPRE, NULL, sfname,
							  FALSE, curbuf, eap);
	else
	    apply_autocmds_exarg(EVENT_FILEREADPRE, sfname, sfname,
							    FALSE, NULL, eap);
	try_mac = (vim_strchr(p_ffs, 'm') != NULL);
	try_dos = (vim_strchr(p_ffs, 'd') != NULL);
	try_unix = (vim_strchr(p_ffs, 'x') != NULL);
	if (msg_scrolled == n)
	    msg_scroll = m;
#ifdef FEAT_EVAL
	if (aborting())	     
	{
	    --no_wait_return;
	    msg_scroll = msg_save;
	    curbuf->b_p_ro = TRUE;	 
	    return FAIL;
	}
#endif
	if (!read_stdin && (curbuf != old_curbuf
		|| (using_b_ffname && (old_b_ffname != curbuf->b_ffname))
		|| (using_b_fname && (old_b_fname != curbuf->b_fname))
		|| (fd = mch_open((char *)fname, O_RDONLY | O_EXTRA, 0)) < 0))
	{
	    --no_wait_return;
	    msg_scroll = msg_save;
	    if (fd < 0)
		EMSG(_("E200: *ReadPre autocommands made the file unreadable"));
	    else
		EMSG(_("E201: *ReadPre autocommands must not change current buffer"));
	    curbuf->b_p_ro = TRUE;	 
	    return FAIL;
	}
    }
#endif  
    wasempty = (curbuf->b_ml.ml_flags & ML_EMPTY);
    if (!recoverymode && !filtering && !(flags & READ_DUMMY))
    {
	if (read_stdin)
	{
#ifndef ALWAYS_USE_GUI
	    mch_msg(_("Vim: Reading from stdin...\n"));
#endif
#ifdef FEAT_GUI
	    if (gui.in_use && !gui.dying && !gui.starting)
	    {
		p = (char_u *)_("Reading from stdin...");
		gui_write(p, (int)STRLEN(p));
	    }
#endif
	}
	else if (!read_buffer)
	    filemess(curbuf, sfname, (char_u *)"", 0);
    }
    msg_scroll = FALSE;			 
    linecnt = curbuf->b_ml.ml_line_count;
#ifdef FEAT_MBYTE
    if (eap != NULL && eap->bad_char != 0)
    {
	bad_char_behavior = eap->bad_char;
	if (set_options)
	    curbuf->b_bad_char = eap->bad_char;
    }
    else
	curbuf->b_bad_char = 0;
    if (eap != NULL && eap->force_enc != 0)
    {
	fenc = enc_canonize(eap->cmd + eap->force_enc);
	fenc_alloced = TRUE;
	keep_dest_enc = TRUE;
    }
    else if (curbuf->b_p_bin)
    {
	fenc = (char_u *)"";		 
	fenc_alloced = FALSE;
    }
    else if (curbuf->b_help)
    {
	char_u	    firstline[80];
	int	    fc;
	fenc = (char_u *)"latin1";
	c = enc_utf8;
	if (!c && !read_stdin)
	{
	    fc = fname[STRLEN(fname) - 1];
	    if (TOLOWER_ASC(fc) == 'x')
	    {
		len = read_eintr(fd, firstline, 80);
		vim_lseek(fd, (off_T)0L, SEEK_SET);
		for (p = firstline; p < firstline + len; ++p)
		    if (*p >= 0x80)
		    {
			c = TRUE;
			break;
		    }
	    }
	}
	if (c)
	{
	    fenc_next = fenc;
	    fenc = (char_u *)"utf-8";
	    if (!enc_utf8)
		keep_dest_enc = TRUE;
	}
	fenc_alloced = FALSE;
    }
    else if (*p_fencs == NUL)
    {
	fenc = curbuf->b_p_fenc;	 
	fenc_alloced = FALSE;
    }
    else
    {
	fenc_next = p_fencs;		 
	fenc = next_fenc(&fenc_next);
	fenc_alloced = TRUE;
    }
#endif
retry:
    if (file_rewind)
    {
	if (read_buffer)
	{
	    read_buf_lnum = 1;
	    read_buf_col = 0;
	}
	else if (read_stdin || vim_lseek(fd, (off_T)0L, SEEK_SET) != 0)
	{
	    error = TRUE;
	    goto failed;
	}
	while (lnum > from)
	    ml_delete(lnum--, FALSE);
	file_rewind = FALSE;
#ifdef FEAT_MBYTE
	if (set_options)
	{
	    curbuf->b_p_bomb = FALSE;
	    curbuf->b_start_bomb = FALSE;
	}
	conv_error = 0;
#endif
    }
    if (keep_fileformat)
	keep_fileformat = FALSE;
    else
    {
	if (eap != NULL && eap->force_ff != 0)
	{
	    fileformat = get_fileformat_force(curbuf, eap);
	    try_unix = try_dos = try_mac = FALSE;
	}
	else if (curbuf->b_p_bin)
	    fileformat = EOL_UNIX;		 
	else if (*p_ffs == NUL)
	    fileformat = get_fileformat(curbuf); 
	else
	    fileformat = EOL_UNKNOWN;		 
    }
#ifdef FEAT_MBYTE
# ifdef USE_ICONV
    if (iconv_fd != (iconv_t)-1)
    {
	iconv_close(iconv_fd);
	iconv_fd = (iconv_t)-1;
    }
# endif
    if (advance_fenc)
    {
	advance_fenc = FALSE;
	if (eap != NULL && eap->force_enc != 0)
	{
	    notconverted = TRUE;
	    conv_error = 0;
	    if (fenc_alloced)
		vim_free(fenc);
	    fenc = (char_u *)"";
	    fenc_alloced = FALSE;
	}
	else
	{
	    if (fenc_alloced)
		vim_free(fenc);
	    if (fenc_next != NULL)
	    {
		fenc = next_fenc(&fenc_next);
		fenc_alloced = (fenc_next != NULL);
	    }
	    else
	    {
		fenc = (char_u *)"";
		fenc_alloced = FALSE;
	    }
	}
	if (tmpname != NULL)
	{
	    mch_remove(tmpname);		 
	    vim_free(tmpname);
	    tmpname = NULL;
	}
    }
    fio_flags = 0;
    converted = need_conversion(fenc);
    if (converted)
    {
	if (STRCMP(fenc, ENC_UCSBOM) == 0)
	    fio_flags = FIO_UCSBOM;
	else if (enc_utf8 || STRCMP(p_enc, "latin1") == 0)
	    fio_flags = get_fio_flags(fenc);
# ifdef WIN3264
	if (fio_flags == 0)
	    fio_flags = get_win_fio_flags(fenc);
# endif
# ifdef MACOS_CONVERT
	if (fio_flags == 0)
	    fio_flags = get_mac_fio_flags(fenc);
# endif
# ifdef USE_ICONV
	if (fio_flags == 0
#  ifdef FEAT_EVAL
		&& !did_iconv
#  endif
		)
	    iconv_fd = (iconv_t)my_iconv_open(
				  enc_utf8 ? (char_u *)"utf-8" : p_enc, fenc);
# endif
# ifdef FEAT_EVAL
	if (fio_flags == 0 && !read_stdin && !read_buffer && *p_ccv != NUL
						    && !read_fifo
#  ifdef USE_ICONV
						    && iconv_fd == (iconv_t)-1
#  endif
		)
	{
#  ifdef USE_ICONV
	    did_iconv = FALSE;
#  endif
	    if (tmpname == NULL)
	    {
		tmpname = readfile_charconvert(fname, fenc, &fd);
		if (tmpname == NULL)
		{
		    advance_fenc = TRUE;
		    if (fd < 0)
		    {
			EMSG(_("E202: Conversion made file unreadable!"));
			error = TRUE;
			goto failed;
		    }
		    goto retry;
		}
	    }
	}
	else
# endif
	{
	    if (fio_flags == 0
# ifdef USE_ICONV
		    && iconv_fd == (iconv_t)-1
# endif
	       )
	    {
		advance_fenc = TRUE;
		goto retry;
	    }
	}
    }
    can_retry = (*fenc != NUL && !read_stdin && !read_fifo && !keep_dest_enc);
#endif
    if (!skip_read)
    {
	linerest = 0;
	filesize = 0;
	skip_count = lines_to_skip;
	read_count = lines_to_read;
#ifdef FEAT_MBYTE
	conv_restlen = 0;
#endif
#ifdef FEAT_PERSISTENT_UNDO
	read_undo_file = (newfile && (flags & READ_KEEP_UNDO) == 0
				  && curbuf->b_ffname != NULL
				  && curbuf->b_p_udf
				  && !filtering
				  && !read_fifo
				  && !read_stdin
				  && !read_buffer);
	if (read_undo_file)
	    sha256_start(&sha_ctx);
#endif
#ifdef FEAT_CRYPT
	if (curbuf->b_cryptstate != NULL)
	{
	    crypt_free_state(curbuf->b_cryptstate);
	    curbuf->b_cryptstate = NULL;
	}
#endif
    }
    while (!error && !got_int)
    {
#if VIM_SIZEOF_INT <= 2
	if (linerest >= 0x7ff0)
	{
	    ++split;
	    *ptr = NL;		     
	    size = 1;
	}
	else
#endif
	{
	    if (!skip_read)
	    {
#if VIM_SIZEOF_INT > 2
# if defined(SSIZE_MAX) && (SSIZE_MAX < 0x10000L)
		size = SSIZE_MAX;		     
# else
		size = 0x10000L;		     
# endif
#else
		size = 0x7ff0L - linerest;	     
#endif
		for ( ; size >= 10; size = (long)((long_u)size >> 1))
		{
		    if ((new_buffer = lalloc((long_u)(size + linerest + 1),
							      FALSE)) != NULL)
			break;
		}
		if (new_buffer == NULL)
		{
		    do_outofmem_msg((long_u)(size * 2 + linerest + 1));
		    error = TRUE;
		    break;
		}
		if (linerest)	 
		    mch_memmove(new_buffer, ptr - linerest, (size_t)linerest);
		vim_free(buffer);
		buffer = new_buffer;
		ptr = buffer + linerest;
		line_start = buffer;
#ifdef FEAT_MBYTE
		real_size = (int)size;
# ifdef USE_ICONV
		if (iconv_fd != (iconv_t)-1)
		    size = size / ICONV_MULT;
		else
# endif
		    if (fio_flags & FIO_LATIN1)
		    size = size / 2;
		else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
		    size = (size * 2 / 3) & ~1;
		else if (fio_flags & FIO_UCS4)
		    size = (size * 2 / 3) & ~3;
		else if (fio_flags == FIO_UCSBOM)
		    size = size / ICONV_MULT;	 
# ifdef WIN3264
		else if (fio_flags & FIO_CODEPAGE)
		    size = size / ICONV_MULT;	 
# endif
# ifdef MACOS_CONVERT
		else if (fio_flags & FIO_MACROMAN)
		    size = size / ICONV_MULT;	 
# endif
#endif
#ifdef FEAT_MBYTE
		if (conv_restlen > 0)
		{
		    mch_memmove(ptr, conv_rest, conv_restlen);
		    ptr += conv_restlen;
		    size -= conv_restlen;
		}
#endif
		if (read_buffer)
		{
		    if (read_buf_lnum > from)
			size = 0;
		    else
		    {
			int	n, ni;
			long	tlen;
			tlen = 0;
			for (;;)
			{
			    p = ml_get(read_buf_lnum) + read_buf_col;
			    n = (int)STRLEN(p);
			    if ((int)tlen + n + 1 > size)
			    {
				n = (int)(size - tlen);
				for (ni = 0; ni < n; ++ni)
				{
				    if (p[ni] == NL)
					ptr[tlen++] = NUL;
				    else
					ptr[tlen++] = p[ni];
				}
				read_buf_col += n;
				break;
			    }
			    else
			    {
				for (ni = 0; ni < n; ++ni)
				{
				    if (p[ni] == NL)
					ptr[tlen++] = NUL;
				    else
					ptr[tlen++] = p[ni];
				}
				ptr[tlen++] = NL;
				read_buf_col = 0;
				if (++read_buf_lnum > from)
				{
				    if (!curbuf->b_p_eol)
					--tlen;
				    size = tlen;
				    break;
				}
			    }
			}
		    }
		}
		else
		{
		    size = read_eintr(fd, ptr, size);
		}
#ifdef FEAT_CRYPT
		if (filesize == 0 && size > 0)
		    cryptkey = check_for_cryptkey(cryptkey, ptr, &size,
						  &filesize, newfile, sfname,
						  &did_ask_for_key);
		if (cryptkey != NULL && curbuf->b_cryptstate != NULL
								   && size > 0)
		{
		    if (crypt_works_inplace(curbuf->b_cryptstate))
		    {
			crypt_decode_inplace(curbuf->b_cryptstate, ptr, size);
		    }
		    else
		    {
			char_u	*newptr = NULL;
			int	decrypted_size;
			decrypted_size = crypt_decode_alloc(
				    curbuf->b_cryptstate, ptr, size, &newptr);
			if (size > 0 && decrypted_size == 0)
			    continue;
			if (linerest == 0)
			{
			    new_buffer = newptr;
			}
			else
			{
			    long_u	new_size;
			    new_size = (long_u)(decrypted_size + linerest + 1);
			    new_buffer = lalloc(new_size, FALSE);
			    if (new_buffer == NULL)
			    {
				do_outofmem_msg(new_size);
				error = TRUE;
				break;
			    }
			    mch_memmove(new_buffer, buffer, linerest);
			    if (newptr != NULL)
				mch_memmove(new_buffer + linerest, newptr,
							      decrypted_size);
			}
			if (new_buffer != NULL)
			{
			    vim_free(buffer);
			    buffer = new_buffer;
			    new_buffer = NULL;
			    line_start = buffer;
			    ptr = buffer + linerest;
			}
			size = decrypted_size;
		    }
		}
#endif
		if (size <= 0)
		{
		    if (size < 0)		     
			error = TRUE;
#ifdef FEAT_MBYTE
		    else if (conv_restlen > 0)
		    {
			if (fio_flags != 0
# ifdef USE_ICONV
				|| iconv_fd != (iconv_t)-1
# endif
			   )
			{
			    if (can_retry)
				goto rewind_retry;
			    if (conv_error == 0)
				conv_error = curbuf->b_ml.ml_line_count
								- linecnt + 1;
			}
			else if (illegal_byte == 0)
			    illegal_byte = curbuf->b_ml.ml_line_count
								- linecnt + 1;
			if (bad_char_behavior == BAD_DROP)
			{
			    *(ptr - conv_restlen) = NUL;
			    conv_restlen = 0;
			}
			else
			{
			    if (bad_char_behavior != BAD_KEEP && (fio_flags != 0
# ifdef USE_ICONV
				    || iconv_fd != (iconv_t)-1
# endif
			       ))
			    {
				while (conv_restlen > 0)
				{
				    *(--ptr) = bad_char_behavior;
				    --conv_restlen;
				}
			    }
			    fio_flags = 0;	 
# ifdef USE_ICONV
			    if (iconv_fd != (iconv_t)-1)
			    {
				iconv_close(iconv_fd);
				iconv_fd = (iconv_t)-1;
			    }
# endif
			}
		    }
#endif
		}
	    }
	    skip_read = FALSE;
#ifdef FEAT_MBYTE
	    if ((filesize == 0
# ifdef FEAT_CRYPT
		   || (cryptkey != NULL
			&& filesize == crypt_get_header_len(
						 crypt_get_method_nr(curbuf)))
# endif
		       )
		    && (fio_flags == FIO_UCSBOM
			|| (!curbuf->b_p_bomb
			    && tmpname == NULL
			    && (*fenc == 'u' || (*fenc == NUL && enc_utf8)))))
	    {
		char_u	*ccname;
		int	blen;
		if (size < 2 || curbuf->b_p_bin)
		    ccname = NULL;
		else
		    ccname = check_for_bom(ptr, size, &blen,
		      fio_flags == FIO_UCSBOM ? FIO_ALL : get_fio_flags(fenc));
		if (ccname != NULL)
		{
		    filesize += blen;
		    size -= blen;
		    mch_memmove(ptr, ptr + blen, (size_t)size);
		    if (set_options)
		    {
			curbuf->b_p_bomb = TRUE;
			curbuf->b_start_bomb = TRUE;
		    }
		}
		if (fio_flags == FIO_UCSBOM)
		{
		    if (ccname == NULL)
		    {
			advance_fenc = TRUE;
		    }
		    else
		    {
			if (fenc_alloced)
			    vim_free(fenc);
			fenc = ccname;
			fenc_alloced = FALSE;
		    }
		    skip_read = TRUE;
		    goto retry;
		}
	    }
	    ptr -= conv_restlen;
	    size += conv_restlen;
	    conv_restlen = 0;
#endif
	    if (size <= 0)
		break;
#ifdef FEAT_MBYTE
# ifdef USE_ICONV
	    if (iconv_fd != (iconv_t)-1)
	    {
		const char	*fromp;
		char		*top;
		size_t		from_size;
		size_t		to_size;
		fromp = (char *)ptr;
		from_size = size;
		ptr += size;
		top = (char *)ptr;
		to_size = real_size - size;
		while ((iconv(iconv_fd, (void *)&fromp, &from_size,
							       &top, &to_size)
			    == (size_t)-1 && ICONV_ERRNO != ICONV_EINVAL)
						  || from_size > CONV_RESTLEN)
		{
		    if (can_retry)
			goto rewind_retry;
		    if (conv_error == 0)
			conv_error = readfile_linenr(linecnt,
							  ptr, (char_u *)top);
		    ++fromp;
		    --from_size;
		    if (bad_char_behavior == BAD_KEEP)
		    {
			*top++ = *(fromp - 1);
			--to_size;
		    }
		    else if (bad_char_behavior != BAD_DROP)
		    {
			*top++ = bad_char_behavior;
			--to_size;
		    }
		}
		if (from_size > 0)
		{
		    mch_memmove(conv_rest, (char_u *)fromp, from_size);
		    conv_restlen = (int)from_size;
		}
		line_start = ptr - linerest;
		mch_memmove(line_start, buffer, (size_t)linerest);
		size = (long)((char_u *)top - ptr);
	    }
# endif
# ifdef WIN3264
	    if (fio_flags & FIO_CODEPAGE)
	    {
		char_u	*src, *dst;
		WCHAR	ucs2buf[3];
		int	ucs2len;
		int	codepage = FIO_GET_CP(fio_flags);
		int	bytelen;
		int	found_bad;
		char	replstr[2];
		if (bad_char_behavior > 0)
		    replstr[0] = bad_char_behavior;
		else
		    replstr[0] = '?';
		replstr[1] = NUL;
		src = ptr + real_size - size;
		mch_memmove(src, ptr, size);
		dst = ptr;
		size = size;
		while (size > 0)
		{
		    found_bad = FALSE;
#  ifdef CP_UTF8	 
		    if (codepage == CP_UTF8)
		    {
			bytelen = (int)utf_ptr2len_len(src, size);
			if (bytelen > size)
			{
			    if (bytelen <= CONV_RESTLEN)
				break;
			    bytelen = size;
			    found_bad = TRUE;
			}
			else
			{
			    int	    u8c = utf_ptr2char(src);
			    if (u8c > 0xffff || (*src >= 0x80 && bytelen == 1))
				found_bad = TRUE;
			    ucs2buf[0] = u8c;
			    ucs2len = 1;
			}
		    }
		    else
#  endif
		    {
			for (bytelen = 1; bytelen <= size && bytelen <= 3;
								    ++bytelen)
			{
			    ucs2len = MultiByteToWideChar(codepage,
							 MB_ERR_INVALID_CHARS,
							 (LPCSTR)src, bytelen,
								   ucs2buf, 3);
			    if (ucs2len > 0)
				break;
			}
			if (ucs2len == 0)
			{
			    if (size == 1)
				break;
			    found_bad = TRUE;
			    bytelen = 1;
			}
		    }
		    if (!found_bad)
		    {
			int	i;
			if (enc_utf8)
			{
			    for (i = 0; i < ucs2len; ++i)
				dst += utf_char2bytes(ucs2buf[i], dst);
			}
			else
			{
			    BOOL	bad = FALSE;
			    int		dstlen;
			    dstlen = WideCharToMultiByte(enc_codepage, 0,
				    (LPCWSTR)ucs2buf, ucs2len,
				    (LPSTR)dst, (int)(src - dst),
				    replstr, &bad);
			    if (bad)
				found_bad = TRUE;
			    else
				dst += dstlen;
			}
		    }
		    if (found_bad)
		    {
			if (can_retry)
			    goto rewind_retry;
			if (conv_error == 0)
			    conv_error = readfile_linenr(linecnt, ptr, dst);
			if (bad_char_behavior != BAD_DROP)
			{
			    if (bad_char_behavior == BAD_KEEP)
			    {
				mch_memmove(dst, src, bytelen);
				dst += bytelen;
			    }
			    else
				*dst++ = bad_char_behavior;
			}
		    }
		    src += bytelen;
		    size -= bytelen;
		}
		if (size > 0)
		{
		    mch_memmove(conv_rest, src, size);
		    conv_restlen = size;
		}
		size = (long)(dst - ptr);
	    }
	    else
# endif
# ifdef MACOS_CONVERT
	    if (fio_flags & FIO_MACROMAN)
	    {
		if (macroman2enc(ptr, &size, real_size) == FAIL)
		    goto rewind_retry;
	    }
	    else
# endif
	    if (fio_flags != 0)
	    {
		int	u8c;
		char_u	*dest;
		char_u	*tail = NULL;
		dest = ptr + real_size;
		if (fio_flags == FIO_LATIN1 || fio_flags == FIO_UTF8)
		{
		    p = ptr + size;
		    if (fio_flags == FIO_UTF8)
		    {
			tail = ptr + size - 1;
			while (tail > ptr && (*tail & 0xc0) == 0x80)
			    --tail;
			if (tail + utf_byte2len(*tail) <= ptr + size)
			    tail = NULL;
			else
			    p = tail;
		    }
		}
		else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
		{
		    p = ptr + (size & ~1);
		    if (size & 1)
			tail = p;
		    if ((fio_flags & FIO_UTF16) && p > ptr)
		    {
			if (fio_flags & FIO_ENDIAN_L)
			{
			    u8c = (*--p << 8);
			    u8c += *--p;
			}
			else
			{
			    u8c = *--p;
			    u8c += (*--p << 8);
			}
			if (u8c >= 0xd800 && u8c <= 0xdbff)
			    tail = p;
			else
			    p += 2;
		    }
		}
		else  
		{
		    p = ptr + (size & ~3);
		    if (size & 3)
			tail = p;
		}
		if (tail != NULL)
		{
		    conv_restlen = (int)((ptr + size) - tail);
		    mch_memmove(conv_rest, (char_u *)tail, conv_restlen);
		    size -= conv_restlen;
		}
		while (p > ptr)
		{
		    if (fio_flags & FIO_LATIN1)
			u8c = *--p;
		    else if (fio_flags & (FIO_UCS2 | FIO_UTF16))
		    {
			if (fio_flags & FIO_ENDIAN_L)
			{
			    u8c = (*--p << 8);
			    u8c += *--p;
			}
			else
			{
			    u8c = *--p;
			    u8c += (*--p << 8);
			}
			if ((fio_flags & FIO_UTF16)
					    && u8c >= 0xdc00 && u8c <= 0xdfff)
			{
			    int u16c;
			    if (p == ptr)
			    {
				if (can_retry)
				    goto rewind_retry;
				if (conv_error == 0)
				    conv_error = readfile_linenr(linecnt,
								      ptr, p);
				if (bad_char_behavior == BAD_DROP)
				    continue;
				if (bad_char_behavior != BAD_KEEP)
				    u8c = bad_char_behavior;
			    }
			    if (fio_flags & FIO_ENDIAN_L)
			    {
				u16c = (*--p << 8);
				u16c += *--p;
			    }
			    else
			    {
				u16c = *--p;
				u16c += (*--p << 8);
			    }
			    u8c = 0x10000 + ((u16c & 0x3ff) << 10)
							      + (u8c & 0x3ff);
			    if (u16c < 0xd800 || u16c > 0xdbff)
			    {
				if (can_retry)
				    goto rewind_retry;
				if (conv_error == 0)
				    conv_error = readfile_linenr(linecnt,
								      ptr, p);
				if (bad_char_behavior == BAD_DROP)
				    continue;
				if (bad_char_behavior != BAD_KEEP)
				    u8c = bad_char_behavior;
			    }
			}
		    }
		    else if (fio_flags & FIO_UCS4)
		    {
			if (fio_flags & FIO_ENDIAN_L)
			{
			    u8c = (unsigned)*--p << 24;
			    u8c += (unsigned)*--p << 16;
			    u8c += (unsigned)*--p << 8;
			    u8c += *--p;
			}
			else	 
			{
			    u8c = *--p;
			    u8c += (unsigned)*--p << 8;
			    u8c += (unsigned)*--p << 16;
			    u8c += (unsigned)*--p << 24;
			}
		    }
		    else     
		    {
			if (*--p < 0x80)
			    u8c = *p;
			else
			{
			    len = utf_head_off(ptr, p);
			    p -= len;
			    u8c = utf_ptr2char(p);
			    if (len == 0)
			    {
				if (can_retry)
				    goto rewind_retry;
				if (conv_error == 0)
				    conv_error = readfile_linenr(linecnt,
								      ptr, p);
				if (bad_char_behavior == BAD_DROP)
				    continue;
				if (bad_char_behavior != BAD_KEEP)
				    u8c = bad_char_behavior;
			    }
			}
		    }
		    if (enc_utf8)	 
		    {
			dest -= utf_char2len(u8c);
			(void)utf_char2bytes(u8c, dest);
		    }
		    else		 
		    {
			--dest;
			if (u8c >= 0x100)
			{
			    if (can_retry)
				goto rewind_retry;
			    if (conv_error == 0)
				conv_error = readfile_linenr(linecnt, ptr, p);
			    if (bad_char_behavior == BAD_DROP)
				++dest;
			    else if (bad_char_behavior == BAD_KEEP)
				*dest = u8c;
			    else if (eap != NULL && eap->bad_char != 0)
				*dest = bad_char_behavior;
			    else
				*dest = 0xBF;
			}
			else
			    *dest = u8c;
		    }
		}
		line_start = dest - linerest;
		mch_memmove(line_start, buffer, (size_t)linerest);
		size = (long)((ptr + real_size) - dest);
		ptr = dest;
	    }
	    else if (enc_utf8 && !curbuf->b_p_bin)
	    {
		int  incomplete_tail = FALSE;
		for (p = ptr; ; ++p)
		{
		    int	 todo = (int)((ptr + size) - p);
		    int	 l;
		    if (todo <= 0)
			break;
		    if (*p >= 0x80)
		    {
			l = utf_ptr2len_len(p, todo);
			if (l > todo && !incomplete_tail)
			{
			    if (p > ptr || filesize > 0)
				incomplete_tail = TRUE;
			    if (p > ptr)
			    {
				conv_restlen = todo;
				mch_memmove(conv_rest, p, conv_restlen);
				size -= conv_restlen;
				break;
			    }
			}
			if (l == 1 || l > todo)
			{
			    if (can_retry && !incomplete_tail)
				break;
# ifdef USE_ICONV
			    if (iconv_fd != (iconv_t)-1 && conv_error == 0)
				conv_error = readfile_linenr(linecnt, ptr, p);
# endif
			    if (conv_error == 0 && illegal_byte == 0)
				illegal_byte = readfile_linenr(linecnt, ptr, p);
			    if (bad_char_behavior == BAD_DROP)
			    {
				mch_memmove(p, p + 1, todo - 1);
				--p;
				--size;
			    }
			    else if (bad_char_behavior != BAD_KEEP)
				*p = bad_char_behavior;
			}
			else
			    p += l - 1;
		    }
		}
		if (p < ptr + size && !incomplete_tail)
		{
rewind_retry:
# if defined(FEAT_EVAL) && defined(USE_ICONV)
		    if (*p_ccv != NUL && iconv_fd != (iconv_t)-1)
			did_iconv = TRUE;
		    else
# endif
			advance_fenc = TRUE;
		    file_rewind = TRUE;
		    goto retry;
		}
	    }
#endif
	    filesize += size;
	    if (fileformat == EOL_UNKNOWN)
	    {
		if (try_dos || try_unix)
		{
		    if (try_mac)
			try_mac = 1;
		    for (p = ptr; p < ptr + size; ++p)
		    {
			if (*p == NL)
			{
			    if (!try_unix
				    || (try_dos && p > ptr && p[-1] == CAR))
				fileformat = EOL_DOS;
			    else
				fileformat = EOL_UNIX;
			    break;
			}
			else if (*p == CAR && try_mac)
			    try_mac++;
		    }
		    if (fileformat == EOL_UNIX && try_mac)
		    {
			try_mac = 1;
			try_unix = 1;
			for (; p >= ptr && *p != CAR; p--)
			    ;
			if (p >= ptr)
			{
			    for (p = ptr; p < ptr + size; ++p)
			    {
				if (*p == NL)
				    try_unix++;
				else if (*p == CAR)
				    try_mac++;
			    }
			    if (try_mac > try_unix)
				fileformat = EOL_MAC;
			}
		    }
		    else if (fileformat == EOL_UNKNOWN && try_mac == 1)
			fileformat = default_fileformat();
		}
		if (fileformat == EOL_UNKNOWN && try_mac)
		    fileformat = EOL_MAC;
		if (fileformat == EOL_UNKNOWN)
		    fileformat = default_fileformat();
		if (set_options)
		    set_fileformat(fileformat, OPT_LOCAL);
	    }
	}
	if (fileformat == EOL_MAC)
	{
	    --ptr;
	    while (++ptr, --size >= 0)
	    {
		if ((c = *ptr) != NUL && c != CAR && c != NL)
		    continue;
		if (c == NUL)
		    *ptr = NL;	 
		else if (c == NL)
		    *ptr = CAR;	 
		else
		{
		    if (skip_count == 0)
		    {
			*ptr = NUL;	     
			len = (colnr_T) (ptr - line_start + 1);
			if (ml_append(lnum, line_start, len, newfile) == FAIL)
			{
			    error = TRUE;
			    break;
			}
#ifdef FEAT_PERSISTENT_UNDO
			if (read_undo_file)
			    sha256_update(&sha_ctx, line_start, len);
#endif
			++lnum;
			if (--read_count == 0)
			{
			    error = TRUE;	 
			    line_start = ptr;	 
			    break;
			}
		    }
		    else
			--skip_count;
		    line_start = ptr + 1;
		}
	    }
	}
	else
	{
	    --ptr;
	    while (++ptr, --size >= 0)
	    {
		if ((c = *ptr) != NUL && c != NL)   
		    continue;
		if (c == NUL)
		    *ptr = NL;	 
		else
		{
		    if (skip_count == 0)
		    {
			*ptr = NUL;		 
			len = (colnr_T)(ptr - line_start + 1);
			if (fileformat == EOL_DOS)
			{
			    if (ptr > line_start && ptr[-1] == CAR)
			    {
				ptr[-1] = NUL;
				--len;
			    }
			    else if (ff_error != EOL_DOS)
			    {
				if (   try_unix
				    && !read_stdin
				    && (read_buffer
					|| vim_lseek(fd, (off_T)0L, SEEK_SET)
									  == 0))
				{
				    fileformat = EOL_UNIX;
				    if (set_options)
					set_fileformat(EOL_UNIX, OPT_LOCAL);
				    file_rewind = TRUE;
				    keep_fileformat = TRUE;
				    goto retry;
				}
				ff_error = EOL_DOS;
			    }
			}
			if (ml_append(lnum, line_start, len, newfile) == FAIL)
			{
			    error = TRUE;
			    break;
			}
#ifdef FEAT_PERSISTENT_UNDO
			if (read_undo_file)
			    sha256_update(&sha_ctx, line_start, len);
#endif
			++lnum;
			if (--read_count == 0)
			{
			    error = TRUE;	     
			    line_start = ptr;	 
			    break;
			}
		    }
		    else
			--skip_count;
		    line_start = ptr + 1;
		}
	    }
	}
	linerest = (long)(ptr - line_start);
	ui_breakcheck();
    }
failed:
    if (error && read_count == 0)
	error = FALSE;
    if (!error
	    && !got_int
	    && linerest != 0
	    && !(!curbuf->b_p_bin
		&& fileformat == EOL_DOS
		&& *line_start == Ctrl_Z
		&& ptr == line_start + 1))
    {
	if (set_options)
	    curbuf->b_p_eol = FALSE;
	*ptr = NUL;
	len = (colnr_T)(ptr - line_start + 1);
	if (ml_append(lnum, line_start, len, newfile) == FAIL)
	    error = TRUE;
	else
	{
#ifdef FEAT_PERSISTENT_UNDO
	    if (read_undo_file)
		sha256_update(&sha_ctx, line_start, len);
#endif
	    read_no_eol_lnum = ++lnum;
	}
    }
    if (set_options)
	save_file_ff(curbuf);		 
#ifdef FEAT_CRYPT
    if (curbuf->b_cryptstate != NULL)
    {
	crypt_free_state(curbuf->b_cryptstate);
	curbuf->b_cryptstate = NULL;
    }
    if (cryptkey != NULL && cryptkey != curbuf->b_p_key)
	crypt_free_key(cryptkey);
#endif
#ifdef FEAT_MBYTE
    if (set_options)
	set_string_option_direct((char_u *)"fenc", -1, fenc,
						       OPT_FREE|OPT_LOCAL, 0);
    if (fenc_alloced)
	vim_free(fenc);
# ifdef USE_ICONV
    if (iconv_fd != (iconv_t)-1)
    {
	iconv_close(iconv_fd);
	iconv_fd = (iconv_t)-1;
    }
# endif
#endif
    if (!read_buffer && !read_stdin)
	close(fd);				 
#ifdef HAVE_FD_CLOEXEC
    else
    {
	int fdflags = fcntl(fd, F_GETFD);
	if (fdflags >= 0 && (fdflags & FD_CLOEXEC) == 0)
	    (void)fcntl(fd, F_SETFD, fdflags | FD_CLOEXEC);
    }
#endif
    vim_free(buffer);
#ifdef HAVE_DUP
    if (read_stdin)
    {
	close(0);
	ignored = dup(2);
    }
#endif
#ifdef FEAT_MBYTE
    if (tmpname != NULL)
    {
	mch_remove(tmpname);		 
	vim_free(tmpname);
    }
#endif
    --no_wait_return;			 
    if (!recoverymode)
    {
	if (newfile && wasempty && !(curbuf->b_ml.ml_flags & ML_EMPTY))
	{
#ifdef FEAT_NETBEANS_INTG
	    netbeansFireChanges = 0;
#endif
	    ml_delete(curbuf->b_ml.ml_line_count, FALSE);
#ifdef FEAT_NETBEANS_INTG
	    netbeansFireChanges = 1;
#endif
	    --linecnt;
	}
	linecnt = curbuf->b_ml.ml_line_count - linecnt;
	if (filesize == 0)
	    linecnt = 0;
	if (newfile || read_buffer)
	{
	    redraw_curbuf_later(NOT_VALID);
#ifdef FEAT_DIFF
	    diff_invalidate(curbuf);
#endif
#ifdef FEAT_FOLDING
	    foldUpdateAll(curwin);
#endif
	}
	else if (linecnt)		 
	    appended_lines_mark(from, linecnt);
#ifndef ALWAYS_USE_GUI
	if (read_stdin)
	{
	    settmode(TMODE_RAW);	 
	    starttermcap();
	    screenclear();
	}
#endif
	if (got_int)
	{
	    if (!(flags & READ_DUMMY))
	    {
		filemess(curbuf, sfname, (char_u *)_(e_interr), 0);
		if (newfile)
		    curbuf->b_p_ro = TRUE;	 
	    }
	    msg_scroll = msg_save;
#ifdef FEAT_VIMINFO
	    check_marks_read();
#endif
	    return OK;		 
	}
	if (!filtering && !(flags & READ_DUMMY))
	{
	    msg_add_fname(curbuf, sfname);    
	    c = FALSE;
#ifdef UNIX
# ifdef S_ISFIFO
	    if (S_ISFIFO(perm))			     
	    {
		STRCAT(IObuff, _("[fifo/socket]"));
		c = TRUE;
	    }
# else
#  ifdef S_IFIFO
	    if ((perm & S_IFMT) == S_IFIFO)	     
	    {
		STRCAT(IObuff, _("[fifo]"));
		c = TRUE;
	    }
#  endif
#  ifdef S_IFSOCK
	    if ((perm & S_IFMT) == S_IFSOCK)	     
	    {
		STRCAT(IObuff, _("[socket]"));
		c = TRUE;
	    }
#  endif
# endif
# ifdef OPEN_CHR_FILES
	    if (S_ISCHR(perm))			     
	    {
		STRCAT(IObuff, _("[character special]"));
		c = TRUE;
	    }
# endif
#endif
	    if (curbuf->b_p_ro)
	    {
		STRCAT(IObuff, shortmess(SHM_RO) ? _("[RO]") : _("[readonly]"));
		c = TRUE;
	    }
	    if (read_no_eol_lnum)
	    {
		msg_add_eol();
		c = TRUE;
	    }
	    if (ff_error == EOL_DOS)
	    {
		STRCAT(IObuff, _("[CR missing]"));
		c = TRUE;
	    }
	    if (split)
	    {
		STRCAT(IObuff, _("[long lines split]"));
		c = TRUE;
	    }
#ifdef FEAT_MBYTE
	    if (notconverted)
	    {
		STRCAT(IObuff, _("[NOT converted]"));
		c = TRUE;
	    }
	    else if (converted)
	    {
		STRCAT(IObuff, _("[converted]"));
		c = TRUE;
	    }
#endif
#ifdef FEAT_CRYPT
	    if (cryptkey != NULL)
	    {
		crypt_append_msg(curbuf);
		c = TRUE;
	    }
#endif
#ifdef FEAT_MBYTE
	    if (conv_error != 0)
	    {
		sprintf((char *)IObuff + STRLEN(IObuff),
		       _("[CONVERSION ERROR in line %ld]"), (long)conv_error);
		c = TRUE;
	    }
	    else if (illegal_byte > 0)
	    {
		sprintf((char *)IObuff + STRLEN(IObuff),
			 _("[ILLEGAL BYTE in line %ld]"), (long)illegal_byte);
		c = TRUE;
	    }
	    else
#endif
		if (error)
	    {
		STRCAT(IObuff, _("[READ ERRORS]"));
		c = TRUE;
	    }
	    if (msg_add_fileformat(fileformat))
		c = TRUE;
#ifdef FEAT_CRYPT
	    if (cryptkey != NULL)
		msg_add_lines(c, (long)linecnt, filesize
			 - crypt_get_header_len(crypt_get_method_nr(curbuf)));
	    else
#endif
		msg_add_lines(c, (long)linecnt, filesize);
	    vim_free(keep_msg);
	    keep_msg = NULL;
	    msg_scrolled_ign = TRUE;
#ifdef ALWAYS_USE_GUI
	    if (read_stdin || read_buffer)
		p = msg_may_trunc(FALSE, IObuff);
	    else
#endif
		p = msg_trunc_attr(IObuff, FALSE, 0);
	    if (read_stdin || read_buffer || restart_edit != 0
		    || (msg_scrolled != 0 && !need_wait_return))
		set_keep_msg(p, 0);
	    msg_scrolled_ign = FALSE;
	}
	if (newfile && (error
#ifdef FEAT_MBYTE
		    || conv_error != 0
		    || (illegal_byte > 0 && bad_char_behavior != BAD_KEEP)
#endif
		    ))
	    curbuf->b_p_ro = TRUE;
	u_clearline();	     
	if (exmode_active)
	    curwin->w_cursor.lnum = from + linecnt;
	else
	    curwin->w_cursor.lnum = from + 1;
	check_cursor_lnum();
	beginline(BL_WHITE | BL_FIX);	     
	curbuf->b_op_start.lnum = from + 1;
	curbuf->b_op_start.col = 0;
	curbuf->b_op_end.lnum = from + linecnt;
	curbuf->b_op_end.col = 0;
#ifdef WIN32
	if (newfile && !read_stdin && !read_buffer
					 && mch_stat((char *)fname, &st) >= 0)
	{
	    buf_store_time(curbuf, &st, fname);
	    curbuf->b_mtime_read = curbuf->b_mtime;
	}
#endif
    }
    msg_scroll = msg_save;
#ifdef FEAT_VIMINFO
    check_marks_read();
#endif
    curbuf->b_no_eol_lnum = read_no_eol_lnum;
    if (flags & READ_KEEP_UNDO)
	u_find_first_changed();
#ifdef FEAT_PERSISTENT_UNDO
    if (read_undo_file)
    {
	char_u	hash[UNDO_HASH_SIZE];
	sha256_finish(&sha_ctx, hash);
	u_read_undo(NULL, hash, fname);
    }
#endif
#ifdef FEAT_AUTOCMD
    if (!read_stdin && !read_fifo && (!read_buffer || sfname != NULL))
    {
	int m = msg_scroll;
	int n = msg_scrolled;
	if (set_options)
	    save_file_ff(curbuf);
	msg_scroll = TRUE;
	if (filtering)
	    apply_autocmds_exarg(EVENT_FILTERREADPOST, NULL, sfname,
							  FALSE, curbuf, eap);
	else if (newfile || (read_buffer && sfname != NULL))
	{
	    apply_autocmds_exarg(EVENT_BUFREADPOST, NULL, sfname,
							  FALSE, curbuf, eap);
	    if (!au_did_filetype && *curbuf->b_p_ft != NUL)
		apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft, curbuf->b_fname,
			TRUE, curbuf);
	}
	else
	    apply_autocmds_exarg(EVENT_FILEREADPOST, sfname, sfname,
							    FALSE, NULL, eap);
	if (msg_scrolled == n)
	    msg_scroll = m;
# ifdef FEAT_EVAL
	if (aborting())	     
	    return FAIL;
# endif
    }
#endif
    if (recoverymode && error)
	return FAIL;
    return OK;
}