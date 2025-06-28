nv_g_cmd(cmdarg_T *cap)
{
    oparg_T	*oap = cap->oap;
    int		i;
    switch (cap->nchar)
    {
    case Ctrl_A:
    case Ctrl_X:
#ifdef MEM_PROFILE
	if (!VIsual_active && cap->nchar == Ctrl_A)
	    vim_mem_profile_dump();
	else
#endif
	     if (VIsual_active)
	{
	    cap->arg = TRUE;
	    cap->cmdchar = cap->nchar;
	    cap->nchar = NUL;
	    nv_addsub(cap);
	}
	else
	    clearopbeep(oap);
	break;
    case 'R':
	cap->arg = TRUE;
	nv_Replace(cap);
	break;
    case 'r':
	nv_vreplace(cap);
	break;
    case '&':
	do_cmdline_cmd((char_u *)"%s//~/&");
	break;
    case 'v':
	nv_gv_cmd(cap);
	break;
    case 'V':
	VIsual_reselect = FALSE;
	break;
    case K_BS:
	cap->nchar = Ctrl_H;
    case 'h':
    case 'H':
    case Ctrl_H:
	cap->cmdchar = cap->nchar + ('v' - 'h');
	cap->arg = TRUE;
	nv_visual(cap);
	break;
    case 'N':
    case 'n':
	if (!current_search(cap->count1, cap->nchar == 'n'))
	    clearopbeep(oap);
	break;
    case 'j':
    case K_DOWN:
	if (!curwin->w_p_wrap)
	{
	    oap->motion_type = MLINE;
	    i = cursor_down(cap->count1, oap->op_type == OP_NOP);
	}
	else
	    i = nv_screengo(oap, FORWARD, cap->count1);
	if (i == FAIL)
	    clearopbeep(oap);
	break;
    case 'k':
    case K_UP:
	if (!curwin->w_p_wrap)
	{
	    oap->motion_type = MLINE;
	    i = cursor_up(cap->count1, oap->op_type == OP_NOP);
	}
	else
	    i = nv_screengo(oap, BACKWARD, cap->count1);
	if (i == FAIL)
	    clearopbeep(oap);
	break;
    case 'J':
	nv_join(cap);
	break;
    case '^':
    case '0':
    case 'm':
    case K_HOME:
    case K_KHOME:
	nv_g_home_m_cmd(cap);
	break;
    case 'M':
	{
	    oap->motion_type = MCHAR;
	    oap->inclusive = FALSE;
	    i = linetabsize(ml_get_curline());
	    if (cap->count0 > 0 && cap->count0 <= 100)
		coladvance((colnr_T)(i * cap->count0 / 100));
	    else
		coladvance((colnr_T)(i / 2));
	    curwin->w_set_curswant = TRUE;
	}
	break;
    case '_':
	nv_g_underscore_cmd(cap);
	break;
    case '$':
    case K_END:
    case K_KEND:
	nv_g_dollar_cmd(cap);
	break;
    case '*':
    case '#':
#if POUND != '#'
    case POUND:		 
#endif
    case Ctrl_RSB:		 
    case ']':			 
	nv_ident(cap);
	break;
    case 'e':
    case 'E':
	oap->motion_type = MCHAR;
	curwin->w_set_curswant = TRUE;
	oap->inclusive = TRUE;
	if (bckend_word(cap->count1, cap->nchar == 'E', FALSE) == FAIL)
	    clearopbeep(oap);
	break;
    case Ctrl_G:
	cursor_pos_info(NULL);
	break;
    case 'i':
	nv_gi_cmd(cap);
	break;
    case 'I':
	beginline(0);
	if (!checkclearopq(oap))
	    invoke_edit(cap, FALSE, 'g', FALSE);
	break;
#ifdef FEAT_SEARCHPATH
    case 'f':
    case 'F':
	nv_gotofile(cap);
	break;
#endif
    case '\'':
	cap->arg = TRUE;
    case '`':
	nv_gomark(cap);
	break;
    case 's':
	do_sleep(cap->count1 * 1000L, FALSE);
	break;
    case 'a':
	do_ascii(NULL);
	break;
    case '8':
	if (cap->count0 == 8)
	    utf_find_illegal();
	else
	    show_utf8();
	break;
    case '<':
	show_sb_text();
	break;
    case 'g':
	cap->arg = FALSE;
	nv_goto(cap);
	break;
    case 'q':
    case 'w':
	oap->cursor_start = curwin->w_cursor;
    case '~':
    case 'u':
    case 'U':
    case '?':
    case '@':
	nv_operator(cap);
	break;
    case 'd':
    case 'D':
	nv_gd(oap, cap->nchar, (int)cap->count0);
	break;
    case K_MIDDLEMOUSE:
    case K_MIDDLEDRAG:
    case K_MIDDLERELEASE:
    case K_LEFTMOUSE:
    case K_LEFTDRAG:
    case K_LEFTRELEASE:
    case K_MOUSEMOVE:
    case K_RIGHTMOUSE:
    case K_RIGHTDRAG:
    case K_RIGHTRELEASE:
    case K_X1MOUSE:
    case K_X1DRAG:
    case K_X1RELEASE:
    case K_X2MOUSE:
    case K_X2DRAG:
    case K_X2RELEASE:
	mod_mask = MOD_MASK_CTRL;
	(void)do_mouse(oap, cap->nchar, BACKWARD, cap->count1, 0);
	break;
    case K_IGNORE:
	break;
    case 'p':
    case 'P':
	nv_put(cap);
	break;
#ifdef FEAT_BYTEOFF
    case 'o':
	goto_byte(cap->count0);
	break;
#endif
    case 'Q':
	if (text_locked())
	{
	    clearopbeep(cap->oap);
	    text_locked_msg();
	    break;
	}
	if (!checkclearopq(oap))
	    do_exmode(TRUE);
	break;
    case ',':
	nv_pcmark(cap);
	break;
    case ';':
	cap->count1 = -cap->count1;
	nv_pcmark(cap);
	break;
    case 't':
	if (!checkclearop(oap))
	    goto_tabpage((int)cap->count0);
	break;
    case 'T':
	if (!checkclearop(oap))
	    goto_tabpage(-(int)cap->count1);
	break;
    case TAB:
	if (!checkclearop(oap) && goto_tabpage_lastused() == FAIL)
	    clearopbeep(oap);
	break;
    case '+':
    case '-':  
	if (!checkclearopq(oap))
	    undo_time(cap->nchar == '-' ? -cap->count1 : cap->count1,
							 FALSE, FALSE, FALSE);
	break;
    default:
	clearopbeep(oap);
	break;
    }
}