normal_cmd(
    oparg_T	*oap,
    int		toplevel UNUSED)	 
{
    cmdarg_T	ca;			 
    int		c;
    int		ctrl_w = FALSE;		 
    int		old_col = curwin->w_curswant;
    int		need_flushbuf = FALSE;	 
    pos_T	old_pos;		 
    int		mapped_len;
    static int	old_mapped_len = 0;
    int		idx;
    int		set_prevcount = FALSE;
    int		save_did_cursorhold = did_cursorhold;
    CLEAR_FIELD(ca);	 
    ca.oap = oap;
    ca.opcount = opcount;
#ifdef CURSOR_SHAPE
    c = finish_op;
#endif
    finish_op = (oap->op_type != OP_NOP);
#ifdef CURSOR_SHAPE
    if (finish_op != c)
    {
	ui_cursor_shape();		 
# ifdef FEAT_MOUSESHAPE
	update_mouseshape(-1);
# endif
    }
#endif
    may_trigger_modechanged();
    if (!finish_op && !oap->regname)
    {
	ca.opcount = 0;
#ifdef FEAT_EVAL
	set_prevcount = TRUE;
#endif
    }
    if (oap->prev_opcount > 0 || oap->prev_count0 > 0)
    {
	ca.opcount = oap->prev_opcount;
	ca.count0 = oap->prev_count0;
	oap->prev_opcount = 0;
	oap->prev_count0 = 0;
    }
    mapped_len = typebuf_maplen();
    State = MODE_NORMAL_BUSY;
#ifdef USE_ON_FLY_SCROLL
    dont_scroll = FALSE;	 
#endif
#ifdef FEAT_EVAL
    if (toplevel && readbuf1_empty())
	set_vcount_ca(&ca, &set_prevcount);
#endif
    c = safe_vgetc();
    LANGMAP_ADJUST(c, get_real_state() != MODE_SELECT);
    if (restart_edit == 0)
	old_mapped_len = 0;
    else if (old_mapped_len
		|| (VIsual_active && mapped_len == 0 && typebuf_maplen() > 0))
	old_mapped_len = typebuf_maplen();
    if (c == NUL)
	c = K_ZERO;
    if (VIsual_active
	    && VIsual_select
	    && (vim_isprintc(c) || c == NL || c == CAR || c == K_KENTER))
    {
	int len;
	len = ins_char_typebuf(vgetc_char, vgetc_mod_mask);
	if (KeyTyped)
	    ungetchars(len);
	if (restart_edit != 0)
	    c = 'd';
	else
	    c = 'c';
	msg_nowait = TRUE;	 
	old_mapped_len = 0;	 
    }
    if (KeyTyped && !KeyStuffed)
	win_ensure_size();
    need_flushbuf = add_to_showcmd(c);
    c = normal_cmd_get_count(&ca, c, toplevel, set_prevcount, &ctrl_w,
							&need_flushbuf);
    if (ctrl_w)
    {
	ca.nchar = c;
	ca.cmdchar = Ctrl_W;
    }
    else
	ca.cmdchar = c;
    idx = find_command(ca.cmdchar);
    if (idx < 0)
    {
	clearopbeep(oap);
	goto normal_end;
    }
    if ((nv_cmds[idx].cmd_flags & NV_NCW)
				&& (check_text_locked(oap) || curbuf_locked()))
	goto normal_end;
    if (VIsual_active)
    {
	if (km_stopsel
		&& (nv_cmds[idx].cmd_flags & NV_STS)
		&& !(mod_mask & MOD_MASK_SHIFT))
	{
	    end_visual_mode();
	    redraw_curbuf_later(UPD_INVERTED);
	}
	if (km_startsel)
	{
	    if (nv_cmds[idx].cmd_flags & NV_SS)
	    {
		unshift_special(&ca);
		idx = find_command(ca.cmdchar);
		if (idx < 0)
		{
		    clearopbeep(oap);
		    goto normal_end;
		}
	    }
	    else if ((nv_cmds[idx].cmd_flags & NV_SSS)
					       && (mod_mask & MOD_MASK_SHIFT))
		mod_mask &= ~MOD_MASK_SHIFT;
	}
    }
#ifdef FEAT_RIGHTLEFT
    if (curwin->w_p_rl && KeyTyped && !KeyStuffed
					  && (nv_cmds[idx].cmd_flags & NV_RL))
    {
	switch (ca.cmdchar)
	{
	    case 'l':	    ca.cmdchar = 'h'; break;
	    case K_RIGHT:   ca.cmdchar = K_LEFT; break;
	    case K_S_RIGHT: ca.cmdchar = K_S_LEFT; break;
	    case K_C_RIGHT: ca.cmdchar = K_C_LEFT; break;
	    case 'h':	    ca.cmdchar = 'l'; break;
	    case K_LEFT:    ca.cmdchar = K_RIGHT; break;
	    case K_S_LEFT:  ca.cmdchar = K_S_RIGHT; break;
	    case K_C_LEFT:  ca.cmdchar = K_C_RIGHT; break;
	    case '>':	    ca.cmdchar = '<'; break;
	    case '<':	    ca.cmdchar = '>'; break;
	}
	idx = find_command(ca.cmdchar);
    }
#endif
    if (normal_cmd_needs_more_chars(&ca, nv_cmds[idx].cmd_flags))
	idx = normal_cmd_get_more_chars(idx, &ca, &need_flushbuf);
    if (need_flushbuf)
	out_flush();
    if (ca.cmdchar != K_IGNORE)
    {
	if (ex_normal_busy)
	    did_cursorhold = save_did_cursorhold;
	else
	    did_cursorhold = FALSE;
    }
    State = MODE_NORMAL;
    if (ca.nchar == ESC)
    {
	clearop(oap);
	if (restart_edit == 0 && goto_im())
	    restart_edit = 'a';
	goto normal_end;
    }
    if (ca.cmdchar != K_IGNORE)
    {
	msg_didout = FALSE;     
	msg_col = 0;
    }
    old_pos = curwin->w_cursor;		 
    if (!VIsual_active && km_startsel)
    {
	if (nv_cmds[idx].cmd_flags & NV_SS)
	{
	    start_selection();
	    unshift_special(&ca);
	    idx = find_command(ca.cmdchar);
	}
	else if ((nv_cmds[idx].cmd_flags & NV_SSS)
					   && (mod_mask & MOD_MASK_SHIFT))
	{
	    start_selection();
	    mod_mask &= ~MOD_MASK_SHIFT;
	}
    }
    ca.arg = nv_cmds[idx].cmd_arg;
    (nv_cmds[idx].cmd_func)(&ca);
    if (!finish_op
	    && !oap->op_type
	    && (idx < 0 || !(nv_cmds[idx].cmd_flags & NV_KEEPREG)))
    {
	clearop(oap);
#ifdef FEAT_EVAL
	reset_reg_var();
#endif
    }
    if (old_mapped_len > 0)
	old_mapped_len = typebuf_maplen();
    if (ca.cmdchar != K_IGNORE && ca.cmdchar != K_MOUSEMOVE)
	do_pending_operator(&ca, old_col, FALSE);
    if (normal_cmd_need_to_wait_for_msg(&ca, &old_pos))
	normal_cmd_wait_for_msg();
normal_end:
    msg_nowait = FALSE;
#ifdef FEAT_EVAL
    if (finish_op)
	reset_reg_var();
#endif
#ifdef CURSOR_SHAPE
    c = finish_op;
#endif
    finish_op = FALSE;
    may_trigger_modechanged();
#ifdef CURSOR_SHAPE
    if (c || ca.cmdchar == 'r')
    {
	ui_cursor_shape();		 
# ifdef FEAT_MOUSESHAPE
	update_mouseshape(-1);
# endif
    }
#endif
    if (oap->op_type == OP_NOP && oap->regname == 0
	    && ca.cmdchar != K_CURSORHOLD)
	clear_showcmd();
    checkpcmark();		 
    vim_free(ca.searchbuf);
    if (has_mbyte)
	mb_adjust_cursor();
    if (curwin->w_p_scb && toplevel)
    {
	validate_cursor();	 
	do_check_scrollbind(TRUE);
    }
    if (curwin->w_p_crb && toplevel)
    {
	validate_cursor();	 
	do_check_cursorbind();
    }
#ifdef FEAT_TERMINAL
    if (term_job_running(curbuf->b_term))
	restart_edit = 0;
#endif
    if (       oap->op_type == OP_NOP
	    && ((restart_edit != 0 && !VIsual_active && old_mapped_len == 0)
		|| restart_VIsual_select == 1)
	    && !(ca.retval & CA_COMMAND_BUSY)
	    && stuff_empty()
	    && oap->regname == 0)
    {
	if (restart_VIsual_select == 1)
	{
	    VIsual_select = TRUE;
	    may_trigger_modechanged();
	    showmode();
	    restart_VIsual_select = 0;
	    VIsual_select_reg = 0;
	}
	if (restart_edit != 0 && !VIsual_active && old_mapped_len == 0)
	    (void)edit(restart_edit, FALSE, 1L);
    }
    if (restart_VIsual_select == 2)
	restart_VIsual_select = 1;
    opcount = ca.opcount;
}