ins_comp_get_next_word_or_line(
	buf_T	*ins_buf,		 
	pos_T	*cur_match_pos,		 
	int	*match_len,
	int	*cont_s_ipos)		 
{
    char_u	*ptr;
    int		len;
    *match_len = 0;
    ptr = ml_get_buf(ins_buf, cur_match_pos->lnum, FALSE) +
	cur_match_pos->col;
    if (ctrl_x_mode_line_or_eval())
    {
	if (compl_status_adding())
	{
	    if (cur_match_pos->lnum >= ins_buf->b_ml.ml_line_count)
		return NULL;
	    ptr = ml_get_buf(ins_buf, cur_match_pos->lnum + 1, FALSE);
	    if (!p_paste)
		ptr = skipwhite(ptr);
	}
	len = (int)STRLEN(ptr);
    }
    else
    {
	char_u	*tmp_ptr = ptr;
	if (compl_status_adding())
	{
	    tmp_ptr += compl_length;
	    if (vim_iswordp(tmp_ptr))
		return NULL;
	    tmp_ptr = find_word_start(tmp_ptr);
	}
	tmp_ptr = find_word_end(tmp_ptr);
	len = (int)(tmp_ptr - ptr);
	if (compl_status_adding() && len == compl_length)
	{
	    if (cur_match_pos->lnum < ins_buf->b_ml.ml_line_count)
	    {
		STRNCPY(IObuff, ptr, len);
		ptr = ml_get_buf(ins_buf, cur_match_pos->lnum + 1, FALSE);
		tmp_ptr = ptr = skipwhite(ptr);
		tmp_ptr = find_word_start(tmp_ptr);
		tmp_ptr = find_word_end(tmp_ptr);
		if (tmp_ptr > ptr)
		{
		    if (*ptr != ')' && IObuff[len - 1] != TAB)
		    {
			if (IObuff[len - 1] != ' ')
			    IObuff[len++] = ' ';
			if (p_js
				&& (IObuff[len - 2] == '.'
				    || (vim_strchr(p_cpo, CPO_JOINSP)
					== NULL
					&& (IObuff[len - 2] == '?'
					    || IObuff[len - 2] == '!'))))
			    IObuff[len++] = ' ';
		    }
		    if (tmp_ptr - ptr >= IOSIZE - len)
			tmp_ptr = ptr + IOSIZE - len - 1;
		    STRNCPY(IObuff + len, ptr, tmp_ptr - ptr);
		    len += (int)(tmp_ptr - ptr);
		    *cont_s_ipos = TRUE;
		}
		IObuff[len] = NUL;
		ptr = IObuff;
	    }
	    if (len == compl_length)
		return NULL;
	}
    }
    *match_len = len;
    return ptr;
}