msg_puts_printf(char_u *str, int maxlen)
{
    char_u	*s = str;
    char_u	*buf = NULL;
    char_u	*p = s;
#ifdef MSWIN
    if (!(silent_mode && p_verbose == 0))
	mch_settmode(TMODE_COOK);	 
#endif
    while ((maxlen < 0 || (int)(s - str) < maxlen) && *s != NUL)
    {
	if (!(silent_mode && p_verbose == 0))
	{
	    if (*s == NL)
	    {
		int n = (int)(s - p);
		buf = alloc(n + 3);
		if (buf != NULL)
		{
		    memcpy(buf, p, n);
		    if (!info_message)
			buf[n++] = CAR;
		    buf[n++] = NL;
		    buf[n++] = NUL;
		    if (info_message)    
			mch_msg((char *)buf);
		    else
			mch_errmsg((char *)buf);
		    vim_free(buf);
		}
		p = s + 1;
	    }
	}
#ifdef FEAT_RIGHTLEFT
	if (cmdmsg_rl)
	{
	    if (*s == CAR || *s == NL)
		msg_col = Columns - 1;
	    else
		--msg_col;
	}
	else
#endif
	{
	    if (*s == CAR || *s == NL)
		msg_col = 0;
	    else
		++msg_col;
	}
	++s;
    }
    if (*p != NUL && !(silent_mode && p_verbose == 0))
    {
	char_u *tofree = NULL;
	if (maxlen > 0 && STRLEN(p) > (size_t)maxlen)
	{
	    tofree = vim_strnsave(p, (size_t)maxlen);
	    p = tofree;
	}
	if (p != NULL)
	{
	    if (info_message)
		mch_msg((char *)p);
	    else
		mch_errmsg((char *)p);
	    vim_free(tofree);
	}
    }
    msg_didout = TRUE;	     
#ifdef MSWIN
    if (!(silent_mode && p_verbose == 0))
	mch_settmode(TMODE_RAW);
#endif
}