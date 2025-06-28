skip_string(char_u *p)
{
    int	    i;
    for ( ; ; ++p)
    {
	if (p[0] == '\'')		     
	{
	    if (p[1] == NUL)		     
		break;
	    i = 2;
	    if (p[1] == '\\' && p[2] != NUL)     
	    {
		++i;
		while (vim_isdigit(p[i - 1]))    
		    ++i;
	    }
	    if (p[i] == '\'')		     
	    {
		p += i;
		continue;
	    }
	}
	else if (p[0] == '"')		     
	{
	    for (++p; p[0]; ++p)
	    {
		if (p[0] == '\\' && p[1] != NUL)
		    ++p;
		else if (p[0] == '"')	     
		    break;
	    }
	    if (p[0] == '"')
		continue;  
	}
	else if (p[0] == 'R' && p[1] == '"')
	{
	    char_u *delim = p + 2;
	    char_u *paren = vim_strchr(delim, '(');
	    if (paren != NULL)
	    {
		size_t delim_len = paren - delim;
		for (p += 3; *p; ++p)
		    if (p[0] == ')' && STRNCMP(p + 1, delim, delim_len) == 0
			    && p[delim_len + 1] == '"')
		    {
			p += delim_len + 1;
			break;
		    }
		if (p[0] == '"')
		    continue;  
	    }
	}
	break;				     
    }
    if (!*p)
	--p;				     
    return p;
}