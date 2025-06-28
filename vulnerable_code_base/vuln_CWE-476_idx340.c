vgetorpeek(int advance)
{
    int		c, c1;
    int		timedout = FALSE;	 
    int		mapdepth = 0;		 
    int		mode_deleted = FALSE;    
    int		new_wcol, new_wrow;
#ifdef FEAT_GUI
    int		shape_changed = FALSE;   
#endif
    int		n;
    int		old_wcol, old_wrow;
    int		wait_tb_len;
    if (vgetc_busy > 0 && ex_normal_busy == 0)
	return NUL;
    ++vgetc_busy;
    if (advance)
    {
	KeyStuffed = FALSE;
	typebuf_was_empty = FALSE;
    }
    init_typebuf();
    start_stuff();
    check_end_reg_executing(advance);
    do
    {
	if (typeahead_char != 0)
	{
	    c = typeahead_char;
	    if (advance)
		typeahead_char = 0;
	}
	else
	    c = read_readbuffers(advance);
	if (c != NUL && !got_int)
	{
	    if (advance)
	    {
		KeyStuffed = TRUE;
	    }
	    if (typebuf.tb_no_abbr_cnt == 0)
		typebuf.tb_no_abbr_cnt = 1;	 
	}
	else
	{
	    for (;;)
	    {
		long	wait_time;
		int	keylen = 0;
		int	showcmd_idx;
		check_end_reg_executing(advance);
		if (typebuf.tb_maplen)
		    line_breakcheck();
		else
		    ui_breakcheck();		 
		if (got_int)
		{
		    c = inchar(typebuf.tb_buf, typebuf.tb_buflen - 1, 0L);
		    if ((c || typebuf.tb_maplen)
				     && (State & (MODE_INSERT | MODE_CMDLINE)))
			c = ESC;
		    else
			c = Ctrl_C;
		    flush_buffers(FLUSH_INPUT);	 
		    if (advance)
		    {
			*typebuf.tb_buf = c;
			gotchars(typebuf.tb_buf, 1);
		    }
		    cmd_silent = FALSE;
		    break;
		}
		else if (typebuf.tb_len > 0)
		{
		    map_result_T result = handle_mapping(
						&keylen, &timedout, &mapdepth);
		    if (result == map_result_retry)
			continue;
		    if (result == map_result_fail)
		    {
			c = -1;
			break;
		    }
		    if (result == map_result_get)
		    {
			c = typebuf.tb_buf[typebuf.tb_off];
			if (advance)	 
			{
			    cmd_silent = (typebuf.tb_silent > 0);
			    if (typebuf.tb_maplen > 0)
				KeyTyped = FALSE;
			    else
			    {
				KeyTyped = TRUE;
				gotchars(typebuf.tb_buf
						 + typebuf.tb_off, 1);
			    }
			    KeyNoremap = typebuf.tb_noremap[
						      typebuf.tb_off];
			    del_typebuf(1, 0);
			}
			break;   
		    }
		}
		c = 0;
		new_wcol = curwin->w_wcol;
		new_wrow = curwin->w_wrow;
		if (	   advance
			&& typebuf.tb_len == 1
			&& typebuf.tb_buf[typebuf.tb_off] == ESC
			&& !no_mapping
			&& kitty_protocol_state != KKPS_ENABLED
			&& ex_normal_busy == 0
			&& typebuf.tb_maplen == 0
			&& (State & MODE_INSERT)
			&& (p_timeout
			    || (keylen == KEYLEN_PART_KEY && p_ttimeout))
			&& (c = inchar(typebuf.tb_buf + typebuf.tb_off
					       + typebuf.tb_len, 3, 25L)) == 0)
		{
		    colnr_T	col = 0;
		    char_u	*ptr;
		    if (mode_displayed)
		    {
			unshowmode(TRUE);
			mode_deleted = TRUE;
		    }
#ifdef FEAT_GUI
		    if (gui.in_use && State != MODE_NORMAL && !cmd_silent)
		    {
			int	    save_State;
			save_State = State;
			State = MODE_NORMAL;
			gui_update_cursor(TRUE, FALSE);
			State = save_State;
			shape_changed = TRUE;
		    }
#endif
		    validate_cursor();
		    old_wcol = curwin->w_wcol;
		    old_wrow = curwin->w_wrow;
		    if (curwin->w_cursor.col != 0)
		    {
			if (curwin->w_wcol > 0)
			{
			    if (did_ai && *skipwhite(ml_get_curline()
						+ curwin->w_cursor.col) == NUL)
			    {
				chartabsize_T cts;
				curwin->w_wcol = 0;
				ptr = ml_get_curline();
				init_chartabsize_arg(&cts, curwin,
					  curwin->w_cursor.lnum, 0, ptr, ptr);
				while (cts.cts_ptr < ptr + curwin->w_cursor.col)
				{
				    if (!VIM_ISWHITE(*cts.cts_ptr))
					curwin->w_wcol = cts.cts_vcol;
				    cts.cts_vcol += lbr_chartabsize(&cts);
				    if (has_mbyte)
					cts.cts_ptr +=
						   (*mb_ptr2len)(cts.cts_ptr);
				    else
					++cts.cts_ptr;
				}
				clear_chartabsize_arg(&cts);
				curwin->w_wrow = curwin->w_cline_row
					   + curwin->w_wcol / curwin->w_width;
				curwin->w_wcol %= curwin->w_width;
				curwin->w_wcol += curwin_col_off();
				col = 0;	 
			    }
			    else
			    {
				--curwin->w_wcol;
				col = curwin->w_cursor.col - 1;
			    }
			}
			else if (curwin->w_p_wrap && curwin->w_wrow)
			{
			    --curwin->w_wrow;
			    curwin->w_wcol = curwin->w_width - 1;
			    col = curwin->w_cursor.col - 1;
			}
			if (has_mbyte && col > 0 && curwin->w_wcol > 0)
			{
			    ptr = ml_get_curline();
			    col -= (*mb_head_off)(ptr, ptr + col);
			    if ((*mb_ptr2cells)(ptr + col) > 1)
				--curwin->w_wcol;
			}
		    }
		    setcursor();
		    out_flush();
		    new_wcol = curwin->w_wcol;
		    new_wrow = curwin->w_wrow;
		    curwin->w_wcol = old_wcol;
		    curwin->w_wrow = old_wrow;
		}
		if (c < 0)
		    continue;	 
		for (n = 1; n <= c; ++n)
		    typebuf.tb_noremap[typebuf.tb_off + n] = RM_YES;
		typebuf.tb_len += c;
		if (typebuf.tb_len >= typebuf.tb_maplen + MAXMAPLEN)
		{
		    timedout = TRUE;
		    continue;
		}
		if (ex_normal_busy > 0)
		{
		    static int tc = 0;
		    if (typebuf.tb_len > 0)
		    {
			timedout = TRUE;
			continue;
		    }
		    if (p_im && (State & MODE_INSERT))
			c = Ctrl_L;
#ifdef FEAT_TERMINAL
		    else if (terminal_is_active())
			c = K_CANCEL;
#endif
		    else if ((State & MODE_CMDLINE)
					     || (cmdwin_type > 0 && tc == ESC))
			c = Ctrl_C;
		    else
			c = ESC;
		    tc = c;
		    if (advance)
			typebuf_was_empty = TRUE;
		    if (pending_exmode_active)
			exmode_active = EXMODE_NORMAL;
		    typebuf.tb_no_abbr_cnt = 0;
		    break;
		}
		if (((State & MODE_INSERT) != 0 || p_lz)
			&& (State & MODE_CMDLINE) == 0
			&& advance && must_redraw != 0 && !need_wait_return)
		{
		    update_screen(0);
		    setcursor();  
		}
		showcmd_idx = 0;
		c1 = 0;
		if (typebuf.tb_len > 0 && advance && !exmode_active)
		{
		    if (((State & (MODE_NORMAL | MODE_INSERT))
						      || State == MODE_LANGMAP)
			    && State != MODE_HITRETURN)
		    {
			if (State & MODE_INSERT
			    && ptr2cells(typebuf.tb_buf + typebuf.tb_off
						   + typebuf.tb_len - 1) == 1)
			{
			    edit_putchar(typebuf.tb_buf[typebuf.tb_off
						+ typebuf.tb_len - 1], FALSE);
			    setcursor();  
			    c1 = 1;
			}
			old_wcol = curwin->w_wcol;
			old_wrow = curwin->w_wrow;
			curwin->w_wcol = new_wcol;
			curwin->w_wrow = new_wrow;
			push_showcmd();
			if (typebuf.tb_len > SHOWCMD_COLS)
			    showcmd_idx = typebuf.tb_len - SHOWCMD_COLS;
			while (showcmd_idx < typebuf.tb_len)
			    (void)add_to_showcmd(
			       typebuf.tb_buf[typebuf.tb_off + showcmd_idx++]);
			curwin->w_wcol = old_wcol;
			curwin->w_wrow = old_wrow;
		    }
		    if ((State & MODE_CMDLINE)
#if defined(FEAT_CRYPT) || defined(FEAT_EVAL)
			    && cmdline_star == 0
#endif
			    && ptr2cells(typebuf.tb_buf + typebuf.tb_off
						   + typebuf.tb_len - 1) == 1)
		    {
			putcmdline(typebuf.tb_buf[typebuf.tb_off
						+ typebuf.tb_len - 1], FALSE);
			c1 = 1;
		    }
		}
		if (typebuf.tb_len == 0)
		    timedout = FALSE;
		if (advance)
		{
		    if (typebuf.tb_len == 0
			    || !(p_timeout
				 || (p_ttimeout && keylen == KEYLEN_PART_KEY)))
			wait_time = -1L;
		    else if (keylen == KEYLEN_PART_KEY && p_ttm >= 0)
			wait_time = p_ttm;
		    else
			wait_time = p_tm;
		}
		else
		    wait_time = 0;
		wait_tb_len = typebuf.tb_len;
		c = inchar(typebuf.tb_buf + typebuf.tb_off + typebuf.tb_len,
			typebuf.tb_buflen - typebuf.tb_off - typebuf.tb_len - 1,
			wait_time);
		if (showcmd_idx != 0)
		    pop_showcmd();
		if (c1 == 1)
		{
		    if (State & MODE_INSERT)
			edit_unputchar();
		    if (State & MODE_CMDLINE)
			unputcmdline();
		    else
			setcursor();	 
		}
		if (c < 0)
		    continue;		 
		if (c == NUL)		 
		{
		    if (!advance)
			break;
		    if (wait_tb_len > 0)	 
		    {
			timedout = TRUE;
			continue;
		    }
		}
		else
		{	     
		    while (typebuf.tb_buf[typebuf.tb_off
						     + typebuf.tb_len] != NUL)
			typebuf.tb_noremap[typebuf.tb_off
						 + typebuf.tb_len++] = RM_YES;
#ifdef HAVE_INPUT_METHOD
		    vgetc_im_active = im_get_status();
#endif
		}
	    }	     
	}	 
    } while ((c < 0 && c != K_CANCEL) || (advance && c == NUL));
    if (advance && p_smd && msg_silent == 0 && (State & MODE_INSERT))
    {
	if (c == ESC && !mode_deleted && !no_mapping && mode_displayed)
	{
	    if (typebuf.tb_len && !KeyTyped)
		redraw_cmdline = TRUE;	     
	    else
		unshowmode(FALSE);
	}
	else if (c != ESC && mode_deleted)
	{
	    if (typebuf.tb_len && !KeyTyped)
		redraw_cmdline = TRUE;	     
	    else
		showmode();
	}
    }
#ifdef FEAT_GUI
    if (gui.in_use && shape_changed)
	gui_update_cursor(TRUE, FALSE);
#endif
    if (timedout && c == ESC)
    {
	char_u nop_buf[3];
	nop_buf[0] = K_SPECIAL;
	nop_buf[1] = KS_EXTRA;
	nop_buf[2] = KE_NOP;
	gotchars(nop_buf, 3);
    }
    --vgetc_busy;
    return c;
}