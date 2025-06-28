str2special(
    char_u	**sp,
    int		from)	 
{
    int			c;
    static char_u	buf[7];
    char_u		*str = *sp;
    int			modifiers = 0;
    int			special = FALSE;
    if (has_mbyte)
    {
	char_u	*p;
	p = mb_unescape(sp);
	if (p != NULL)
	    return p;
    }
    c = *str;
    if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL)
    {
	if (str[1] == KS_MODIFIER)
	{
	    modifiers = str[2];
	    str += 3;
	    c = *str;
	}
	if (c == K_SPECIAL && str[1] != NUL && str[2] != NUL)
	{
	    c = TO_SPECIAL(str[1], str[2]);
	    str += 2;
	}
	if (IS_SPECIAL(c) || modifiers)	 
	    special = TRUE;
    }
    if (has_mbyte && !IS_SPECIAL(c) && MB_BYTE2LEN(c) > 1)
    {
	char_u	*p;
	*sp = str;
	p = mb_unescape(sp);
	if (p != NULL)
	    c = (*mb_ptr2char)(p);
	else
	    *sp = str + 1;
    }
    else
	*sp = str + 1;
    if (special || c < ' ' || (from && c == ' '))
	return get_special_key_name(c, modifiers);
    buf[0] = c;
    buf[1] = NUL;
    return buf;
}