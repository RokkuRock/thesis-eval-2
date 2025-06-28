ins_compl_infercase_gettext(
	char_u	*str,
	int	actual_len,
	int	actual_compl_length,
	int	min_len)
{
    int		*wca;			 
    char_u	*p;
    int		i, c;
    int		has_lower = FALSE;
    int		was_letter = FALSE;
    IObuff[0] = NUL;
    wca = ALLOC_MULT(int, actual_len);
    if (wca == NULL)
	return IObuff;
    p = str;
    for (i = 0; i < actual_len; ++i)
	if (has_mbyte)
	    wca[i] = mb_ptr2char_adv(&p);
	else
	    wca[i] = *(p++);
    p = compl_orig_text;
    for (i = 0; i < min_len; ++i)
    {
	if (has_mbyte)
	    c = mb_ptr2char_adv(&p);
	else
	    c = *(p++);
	if (MB_ISLOWER(c))
	{
	    has_lower = TRUE;
	    if (MB_ISUPPER(wca[i]))
	    {
		for (i = actual_compl_length; i < actual_len; ++i)
		    wca[i] = MB_TOLOWER(wca[i]);
		break;
	    }
	}
    }
    if (!has_lower)
    {
	p = compl_orig_text;
	for (i = 0; i < min_len; ++i)
	{
	    if (has_mbyte)
		c = mb_ptr2char_adv(&p);
	    else
		c = *(p++);
	    if (was_letter && MB_ISUPPER(c) && MB_ISLOWER(wca[i]))
	    {
		for (i = actual_compl_length; i < actual_len; ++i)
		    wca[i] = MB_TOUPPER(wca[i]);
		break;
	    }
	    was_letter = MB_ISLOWER(c) || MB_ISUPPER(c);
	}
    }
    p = compl_orig_text;
    for (i = 0; i < min_len; ++i)
    {
	if (has_mbyte)
	    c = mb_ptr2char_adv(&p);
	else
	    c = *(p++);
	if (MB_ISLOWER(c))
	    wca[i] = MB_TOLOWER(wca[i]);
	else if (MB_ISUPPER(c))
	    wca[i] = MB_TOUPPER(wca[i]);
    }
    p = IObuff;
    i = 0;
    while (i < actual_len && (p - IObuff + 6) < IOSIZE)
	if (has_mbyte)
	    p += (*mb_char2bytes)(wca[i++], p);
	else
	    *(p++) = wca[i++];
    *p = NUL;
    vim_free(wca);
    return IObuff;
}