regtilde(char_u *source, int magic)
{
    char_u	*newsub = source;
    char_u	*tmpsub;
    char_u	*p;
    int		len;
    int		prevlen;
    for (p = newsub; *p; ++p)
    {
	if ((*p == '~' && magic) || (*p == '\\' && *(p + 1) == '~' && !magic))
	{
	    if (reg_prev_sub != NULL)
	    {
		prevlen = (int)STRLEN(reg_prev_sub);
		tmpsub = alloc(STRLEN(newsub) + prevlen);
		if (tmpsub != NULL)
		{
		    len = (int)(p - newsub);	 
		    mch_memmove(tmpsub, newsub, (size_t)len);
		    mch_memmove(tmpsub + len, reg_prev_sub, (size_t)prevlen);
		    if (!magic)
			++p;			 
		    STRCPY(tmpsub + len + prevlen, p + 1);
		    if (newsub != source)	 
			vim_free(newsub);
		    newsub = tmpsub;
		    p = newsub + len + prevlen;
		}
	    }
	    else if (magic)
		STRMOVE(p, p + 1);	 
	    else
		STRMOVE(p, p + 2);	 
	    --p;
	}
	else
	{
	    if (*p == '\\' && p[1])		 
		++p;
	    if (has_mbyte)
		p += (*mb_ptr2len)(p) - 1;
	}
    }
    vim_free(reg_prev_sub);
    reg_prev_sub = vim_strsave(newsub);
    return newsub;
}