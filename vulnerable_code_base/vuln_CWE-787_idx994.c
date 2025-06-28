do_addsub(
    int		op_type,
    pos_T	*pos,
    int		length,
    linenr_T	Prenum1)
{
    int		col;
    char_u	*buf1;
    char_u	buf2[NUMBUFLEN];
    int		pre;		 
    static int	hexupper = FALSE;	 
    uvarnumber_T	n;
    uvarnumber_T	oldn;
    char_u	*ptr;
    int		c;
    int		todel;
    int		do_hex;
    int		do_oct;
    int		do_bin;
    int		do_alpha;
    int		do_unsigned;
    int		firstdigit;
    int		subtract;
    int		negative = FALSE;
    int		was_positive = TRUE;
    int		visual = VIsual_active;
    int		did_change = FALSE;
    pos_T	save_cursor = curwin->w_cursor;
    int		maxlen = 0;
    pos_T	startpos;
    pos_T	endpos;
    colnr_T	save_coladd = 0;
    do_hex = (vim_strchr(curbuf->b_p_nf, 'x') != NULL);	 
    do_oct = (vim_strchr(curbuf->b_p_nf, 'o') != NULL);	 
    do_bin = (vim_strchr(curbuf->b_p_nf, 'b') != NULL);	 
    do_alpha = (vim_strchr(curbuf->b_p_nf, 'p') != NULL);	 
    do_unsigned = (vim_strchr(curbuf->b_p_nf, 'u') != NULL);	 
    if (virtual_active())
    {
	save_coladd = pos->coladd;
	pos->coladd = 0;
    }
    curwin->w_cursor = *pos;
    ptr = ml_get(pos->lnum);
    col = pos->col;
    if (*ptr == NUL || col + !!save_coladd >= (int)STRLEN(ptr))
	goto theend;
    if (!VIsual_active)
    {
	if (do_bin)
	    while (col > 0 && vim_isbdigit(ptr[col]))
	    {
		--col;
		if (has_mbyte)
		    col -= (*mb_head_off)(ptr, ptr + col);
	    }
	if (do_hex)
	    while (col > 0 && vim_isxdigit(ptr[col]))
	    {
		--col;
		if (has_mbyte)
		    col -= (*mb_head_off)(ptr, ptr + col);
	    }
	if (       do_bin
		&& do_hex
		&& ! ((col > 0
		    && (ptr[col] == 'X'
			|| ptr[col] == 'x')
		    && ptr[col - 1] == '0'
		    && (!has_mbyte ||
			!(*mb_head_off)(ptr, ptr + col - 1))
		    && vim_isxdigit(ptr[col + 1]))))
	{
	    col = pos->col;
	    while (col > 0 && vim_isdigit(ptr[col]))
	    {
		col--;
		if (has_mbyte)
		    col -= (*mb_head_off)(ptr, ptr + col);
	    }
	}
	if ((       do_hex
		&& col > 0
		&& (ptr[col] == 'X'
		    || ptr[col] == 'x')
		&& ptr[col - 1] == '0'
		&& (!has_mbyte ||
		    !(*mb_head_off)(ptr, ptr + col - 1))
		&& vim_isxdigit(ptr[col + 1])) ||
	    (       do_bin
		&& col > 0
		&& (ptr[col] == 'B'
		    || ptr[col] == 'b')
		&& ptr[col - 1] == '0'
		&& (!has_mbyte ||
		    !(*mb_head_off)(ptr, ptr + col - 1))
		&& vim_isbdigit(ptr[col + 1])))
	{
	    --col;
	    if (has_mbyte)
		col -= (*mb_head_off)(ptr, ptr + col);
	}
	else
	{
	    col = pos->col;
	    while (ptr[col] != NUL
		    && !vim_isdigit(ptr[col])
		    && !(do_alpha && ASCII_ISALPHA(ptr[col])))
		col += mb_ptr2len(ptr + col);
	    while (col > 0
		    && vim_isdigit(ptr[col - 1])
		    && !(do_alpha && ASCII_ISALPHA(ptr[col])))
	    {
		--col;
		if (has_mbyte)
		    col -= (*mb_head_off)(ptr, ptr + col);
	    }
	}
    }
    if (visual)
    {
	while (ptr[col] != NUL && length > 0
		&& !vim_isdigit(ptr[col])
		&& !(do_alpha && ASCII_ISALPHA(ptr[col])))
	{
	    int mb_len = mb_ptr2len(ptr + col);
	    col += mb_len;
	    length -= mb_len;
	}
	if (length == 0)
	    goto theend;
	if (col > pos->col && ptr[col - 1] == '-'
		&& (!has_mbyte || !(*mb_head_off)(ptr, ptr + col - 1))
		&& !do_unsigned)
	{
	    negative = TRUE;
	    was_positive = FALSE;
	}
    }
    firstdigit = ptr[col];
    if (!VIM_ISDIGIT(firstdigit) && !(do_alpha && ASCII_ISALPHA(firstdigit)))
    {
	beep_flush();
	goto theend;
    }
    if (do_alpha && ASCII_ISALPHA(firstdigit))
    {
	if (op_type == OP_NR_SUB)
	{
	    if (CharOrd(firstdigit) < Prenum1)
	    {
		if (isupper(firstdigit))
		    firstdigit = 'A';
		else
		    firstdigit = 'a';
	    }
	    else
		firstdigit -= Prenum1;
	}
	else
	{
	    if (26 - CharOrd(firstdigit) - 1 < Prenum1)
	    {
		if (isupper(firstdigit))
		    firstdigit = 'Z';
		else
		    firstdigit = 'z';
	    }
	    else
		firstdigit += Prenum1;
	}
	curwin->w_cursor.col = col;
	if (!did_change)
	    startpos = curwin->w_cursor;
	did_change = TRUE;
	(void)del_char(FALSE);
	ins_char(firstdigit);
	endpos = curwin->w_cursor;
	curwin->w_cursor.col = col;
    }
    else
    {
	pos_T	save_pos;
	int	i;
	if (col > 0 && ptr[col - 1] == '-'
		&& (!has_mbyte ||
		    !(*mb_head_off)(ptr, ptr + col - 1))
		&& !visual
		&& !do_unsigned)
	{
	    --col;
	    negative = TRUE;
	}
	if (visual && VIsual_mode != 'V')
	    maxlen = (curbuf->b_visual.vi_curswant == MAXCOL
		    ? (int)STRLEN(ptr) - col
		    : length);
	int overflow = FALSE;
	vim_str2nr(ptr + col, &pre, &length,
		0 + (do_bin ? STR2NR_BIN : 0)
		    + (do_oct ? STR2NR_OCT : 0)
		    + (do_hex ? STR2NR_HEX : 0),
		NULL, &n, maxlen, FALSE, &overflow);
	if (pre && negative)
	{
	    ++col;
	    --length;
	    negative = FALSE;
	}
	subtract = FALSE;
	if (op_type == OP_NR_SUB)
	    subtract ^= TRUE;
	if (negative)
	    subtract ^= TRUE;
	oldn = n;
	if (!overflow)   
	{
	    if (subtract)
		n -= (uvarnumber_T)Prenum1;
	    else
		n += (uvarnumber_T)Prenum1;
	}
	if (!pre)
	{
	    if (subtract)
	    {
		if (n > oldn)
		{
		    n = 1 + (n ^ (uvarnumber_T)-1);
		    negative ^= TRUE;
		}
	    }
	    else
	    {
		if (n < oldn)
		{
		    n = (n ^ (uvarnumber_T)-1);
		    negative ^= TRUE;
		}
	    }
	    if (n == 0)
		negative = FALSE;
	}
	if (do_unsigned && negative)
	{
	    if (subtract)
		n = (uvarnumber_T)0;
	    else
		n = (uvarnumber_T)(-1);
	    negative = FALSE;
	}
	if (visual && !was_positive && !negative && col > 0)
	{
	    col--;
	    length++;
	}
	curwin->w_cursor.col = col;
	if (!did_change)
	    startpos = curwin->w_cursor;
	did_change = TRUE;
	todel = length;
	c = gchar_cursor();
	if (c == '-')
	    --length;
	save_pos = curwin->w_cursor;
	for (i = 0; i < todel; ++i)
	{
	    if (c < 0x100 && isalpha(c))
	    {
		if (isupper(c))
		    hexupper = TRUE;
		else
		    hexupper = FALSE;
	    }
	    inc_cursor();
	    c = gchar_cursor();
	}
	curwin->w_cursor = save_pos;
	buf1 = alloc(length + NUMBUFLEN);
	if (buf1 == NULL)
	    goto theend;
	ptr = buf1;
	if (negative && (!visual || was_positive))
	    *ptr++ = '-';
	if (pre)
	{
	    *ptr++ = '0';
	    --length;
	}
	if (pre == 'b' || pre == 'B' ||
	    pre == 'x' || pre == 'X')
	{
	    *ptr++ = pre;
	    --length;
	}
	if (pre == 'b' || pre == 'B')
	{
	    int bit = 0;
	    int bits = sizeof(uvarnumber_T) * 8;
	    for (bit = bits; bit > 0; bit--)
		if ((n >> (bit - 1)) & 0x1) break;
	    for (i = 0; bit > 0; bit--)
		buf2[i++] = ((n >> (bit - 1)) & 0x1) ? '1' : '0';
	    buf2[i] = '\0';
	}
	else if (pre == 0)
	    vim_snprintf((char *)buf2, NUMBUFLEN, "%llu", n);
	else if (pre == '0')
	    vim_snprintf((char *)buf2, NUMBUFLEN, "%llo", n);
	else if (pre && hexupper)
	    vim_snprintf((char *)buf2, NUMBUFLEN, "%llX", n);
	else
	    vim_snprintf((char *)buf2, NUMBUFLEN, "%llx", n);
	length -= (int)STRLEN(buf2);
	if (firstdigit == '0' && !(do_oct && pre == 0))
	    while (length-- > 0)
		*ptr++ = '0';
	*ptr = NUL;
	STRCAT(buf1, buf2);
	save_pos = curwin->w_cursor;
	if (todel > 0)
	    inc_cursor();
	ins_str(buf1);		 
	vim_free(buf1);
	if (todel > 0)
	{
	    int bytes_after = (int)STRLEN(ml_get_curline())
							- curwin->w_cursor.col;
	    curwin->w_cursor = save_pos;
	    (void)del_char(FALSE);
	    curwin->w_cursor.col = (colnr_T)(STRLEN(ml_get_curline())
								- bytes_after);
	    --todel;
	}
	while (todel-- > 0)
	    (void)del_char(FALSE);
	endpos = curwin->w_cursor;
	if (did_change && curwin->w_cursor.col)
	    --curwin->w_cursor.col;
    }
    if (did_change && (cmdmod.cmod_flags & CMOD_LOCKMARKS) == 0)
    {
	curbuf->b_op_start = startpos;
	curbuf->b_op_end = endpos;
	if (curbuf->b_op_end.col > 0)
	    --curbuf->b_op_end.col;
    }
theend:
    if (visual)
	curwin->w_cursor = save_cursor;
    else if (did_change)
	curwin->w_set_curswant = TRUE;
    else if (virtual_active())
	curwin->w_cursor.coladd = save_coladd;
    return did_change;
}