do_buffer_ext(
    int		action,
    int		start,
    int		dir,		 
    int		count,		 
    int		flags)		 
{
    buf_T	*buf;
    buf_T	*bp;
    int		unload = (action == DOBUF_UNLOAD || action == DOBUF_DEL
			|| action == DOBUF_WIPE || action == DOBUF_WIPE_REUSE);
    switch (start)
    {
	case DOBUF_FIRST:   buf = firstbuf; break;
	case DOBUF_LAST:    buf = lastbuf;  break;
	default:	    buf = curbuf;   break;
    }
    if (start == DOBUF_MOD)	     
    {
	while (count-- > 0)
	{
	    do
	    {
		buf = buf->b_next;
		if (buf == NULL)
		    buf = firstbuf;
	    }
	    while (buf != curbuf && !bufIsChanged(buf));
	}
	if (!bufIsChanged(buf))
	{
	    emsg(_(e_no_modified_buffer_found));
	    return FAIL;
	}
    }
    else if (start == DOBUF_FIRST && count)  
    {
	while (buf != NULL && buf->b_fnum != count)
	    buf = buf->b_next;
    }
    else
    {
	bp = NULL;
	while (count > 0 || (!unload && !buf->b_p_bl && bp != buf))
	{
	    if (bp == NULL)
		bp = buf;
	    if (dir == FORWARD)
	    {
		buf = buf->b_next;
		if (buf == NULL)
		    buf = firstbuf;
	    }
	    else
	    {
		buf = buf->b_prev;
		if (buf == NULL)
		    buf = lastbuf;
	    }
	    if (unload || buf->b_p_bl)
	    {
		 --count;
		 bp = NULL;	 
	    }
	    if (bp == buf)
	    {
		emsg(_(e_there_is_no_listed_buffer));
		return FAIL;
	    }
	}
    }
    if (buf == NULL)	     
    {
	if (start == DOBUF_FIRST)
	{
	    if (!unload)
		semsg(_(e_buffer_nr_does_not_exist), count);
	}
	else if (dir == FORWARD)
	    emsg(_(e_cannot_go_beyond_last_buffer));
	else
	    emsg(_(e_cannot_go_before_first_buffer));
	return FAIL;
    }
#ifdef FEAT_PROP_POPUP
    if ((flags & DOBUF_NOPOPUP) && bt_popup(buf)
# ifdef FEAT_TERMINAL
				&& !bt_terminal(buf)
#endif
       )
	return OK;
#endif
#ifdef FEAT_GUI
    need_mouse_correct = TRUE;
#endif
    if (unload)
    {
	int	forward;
	bufref_T bufref;
	if (!can_unload_buffer(buf))
	    return FAIL;
	set_bufref(&bufref, buf);
	if (action != DOBUF_WIPE && action != DOBUF_WIPE_REUSE
				   && buf->b_ml.ml_mfp == NULL && !buf->b_p_bl)
	    return FAIL;
	if ((flags & DOBUF_FORCEIT) == 0 && bufIsChanged(buf))
	{
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
	    if ((p_confirm || (cmdmod.cmod_flags & CMOD_CONFIRM)) && p_write)
	    {
		dialog_changed(buf, FALSE);
		if (!bufref_valid(&bufref))
		    return FAIL;
		if (bufIsChanged(buf))
		    return FAIL;
	    }
	    else
#endif
	    {
		semsg(_(e_no_write_since_last_change_for_buffer_nr_add_bang_to_override),
								 buf->b_fnum);
		return FAIL;
	    }
	}
	if (buf == curbuf && VIsual_active)
	    end_visual_mode();
	FOR_ALL_BUFFERS(bp)
	    if (bp->b_p_bl && bp != buf)
		break;
	if (bp == NULL && buf == curbuf)
	    return empty_curbuf(TRUE, (flags & DOBUF_FORCEIT), action);
	while (buf == curbuf
		   && !(curwin->w_closing || curwin->w_buffer->b_locked > 0)
		   && (!ONE_WINDOW || first_tabpage->tp_next != NULL))
	{
	    if (win_close(curwin, FALSE) == FAIL)
		break;
	}
	if (buf != curbuf)
	{
	    close_windows(buf, FALSE);
	    if (buf != curbuf && bufref_valid(&bufref) && buf->b_nwindows <= 0)
		    close_buffer(NULL, buf, action, FALSE, FALSE);
	    return OK;
	}
	buf = NULL;	 
	bp = NULL;	 
	if (au_new_curbuf.br_buf != NULL && bufref_valid(&au_new_curbuf))
	    buf = au_new_curbuf.br_buf;
	else if (curwin->w_jumplistlen > 0)
	{
	    int     jumpidx;
	    jumpidx = curwin->w_jumplistidx - 1;
	    if (jumpidx < 0)
		jumpidx = curwin->w_jumplistlen - 1;
	    forward = jumpidx;
	    while (jumpidx != curwin->w_jumplistidx)
	    {
		buf = buflist_findnr(curwin->w_jumplist[jumpidx].fmark.fnum);
		if (buf != NULL)
		{
		    if (buf == curbuf || !buf->b_p_bl)
			buf = NULL;	 
		    else if (buf->b_ml.ml_mfp == NULL)
		    {
			if (bp == NULL)
			    bp = buf;
			buf = NULL;
		    }
		}
		if (buf != NULL)    
		    break;
		if (!jumpidx && curwin->w_jumplistidx == curwin->w_jumplistlen)
		    break;
		if (--jumpidx < 0)
		    jumpidx = curwin->w_jumplistlen - 1;
		if (jumpidx == forward)		 
		    break;
	    }
	}
	if (buf == NULL)	 
	{
	    forward = TRUE;
	    buf = curbuf->b_next;
	    for (;;)
	    {
		if (buf == NULL)
		{
		    if (!forward)	 
			break;
		    buf = curbuf->b_prev;
		    forward = FALSE;
		    continue;
		}
		if (buf->b_help == curbuf->b_help && buf->b_p_bl)
		{
		    if (buf->b_ml.ml_mfp != NULL)    
			break;
		    if (bp == NULL)	 
			bp = buf;
		}
		if (forward)
		    buf = buf->b_next;
		else
		    buf = buf->b_prev;
	    }
	}
	if (buf == NULL)	 
	    buf = bp;
	if (buf == NULL)	 
	{
	    FOR_ALL_BUFFERS(buf)
		if (buf->b_p_bl && buf != curbuf)
		    break;
	}
	if (buf == NULL)	 
	{
	    if (curbuf->b_next != NULL)
		buf = curbuf->b_next;
	    else
		buf = curbuf->b_prev;
	}
    }
    if (buf == NULL)
    {
	return empty_curbuf(FALSE, (flags & DOBUF_FORCEIT), action);
    }
    if (action == DOBUF_SPLIT)	     
    {
	if ((swb_flags & SWB_USEOPEN) && buf_jump_open_win(buf))
	    return OK;
	if ((swb_flags & SWB_USETAB) && buf_jump_open_tab(buf))
	    return OK;
	if (win_split(0, 0) == FAIL)
	    return FAIL;
    }
    if (buf == curbuf)
	return OK;
    if (action == DOBUF_GOTO && !can_abandon(curbuf, (flags & DOBUF_FORCEIT)))
    {
#if defined(FEAT_GUI_DIALOG) || defined(FEAT_CON_DIALOG)
	if ((p_confirm || (cmdmod.cmod_flags & CMOD_CONFIRM)) && p_write)
	{
	    bufref_T bufref;
	    set_bufref(&bufref, buf);
	    dialog_changed(curbuf, FALSE);
	    if (!bufref_valid(&bufref))
		return FAIL;
	}
	if (bufIsChanged(curbuf))
#endif
	{
	    no_write_message();
	    return FAIL;
	}
    }
    set_curbuf(buf, action);
    if (action == DOBUF_SPLIT)
	RESET_BINDING(curwin);	 
#if defined(FEAT_EVAL)
    if (aborting())	     
	return FAIL;
#endif
    return OK;
}