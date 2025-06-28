_pdfioTokenRead(_pdfio_token_t *tb,	 
		char           *buffer,	 
		size_t         bufsize)	 
{
  int	ch,				 
	parens = 0;			 
  char	*bufptr,			 
	*bufend,			 
	state = '\0';			 
  bool	saw_nul = false;		 
  bufptr = buffer;
  bufend = buffer + bufsize - 1;
  while ((ch = get_char(tb)) != EOF)
  {
    if (ch == '%')
    {
      while ((ch = get_char(tb)) != EOF)
      {
	if (ch == '\n' || ch == '\r')
	  break;
      }
    }
    else if (!isspace(ch))
      break;
  }
  if (ch == EOF)
    return (false);
  if (strchr(PDFIO_DELIM_CHARS, ch) != NULL)
  {
    *bufptr++ = state = (char)ch;
  }
  else if (strchr(PDFIO_NUMBER_CHARS, ch) != NULL)
  {
    state     = 'N';
    *bufptr++ = (char)ch;
  }
  else
  {
    state     = 'K';
    *bufptr++ = (char)ch;
  }
  switch (state)
  {
    case '(' :  
	while ((ch = get_char(tb)) != EOF)
	{
	  if (ch == 0)
	    saw_nul = true;
	  if (ch == '\\')
	  {
	    int	i;			 
	    switch (ch = get_char(tb))
	    {
	      case '0' :  
	      case '1' :
	      case '2' :
	      case '3' :
	      case '4' :
	      case '5' :
	      case '6' :
	      case '7' :
		  for (ch -= '0', i = 0; i < 2; i ++)
		  {
		    int tch = get_char(tb);	 
		    if (tch >= '0' && tch <= '7')
		    {
		      ch = (char)((ch << 3) | (tch - '0'));
		    }
		    else
		    {
		      tb->bufptr --;
		      break;
		    }
		  }
		  break;
	      case '\\' :
	      case '(' :
	      case ')' :
		  break;
	      case 'n' :
		  ch = '\n';
		  break;
	      case 'r' :
		  ch = '\r';
		  break;
	      case 't' :
		  ch = '\t';
		  break;
	      case 'b' :
		  ch = '\b';
		  break;
	      case 'f' :
		  ch = '\f';
		  break;
	      default :
	          break;
	    }
	  }
	  else if (ch == '(')
	  {
	    parens ++;
	  }
	  else if (ch == ')')
	  {
	    if (parens == 0)
	      break;
	    parens --;
	  }
	  if (bufptr < bufend)
	  {
	    *bufptr++ = (char)ch;
	  }
	  else
	  {
	    _pdfioFileError(tb->pdf, "Token too large.");
	    return (false);
	  }
	}
	if (ch != ')')
	{
	  _pdfioFileError(tb->pdf, "Unterminated string literal.");
	  return (false);
	}
	if (saw_nul)
	{
	  char	*litptr,		 
		*hexptr;		 
	  size_t bytes = (size_t)(bufptr - buffer - 1);
          static const char *hexchars = "0123456789ABCDEF";
          PDFIO_DEBUG("_pdfioTokenRead: Converting nul-containing string to binary.\n");
          if ((2 * (bytes + 1)) > bufsize)
          {
	    _pdfioFileError(tb->pdf, "Token too large.");
	    return (false);
          }
	  *buffer = '<';
	  for (litptr = bufptr - 1, hexptr = buffer + 2 * bytes - 1; litptr > buffer; litptr --, hexptr -= 2)
	  {
	    int litch = *litptr;	 
	    hexptr[0] = hexchars[(litch >> 4) & 15];
	    hexptr[1] = hexchars[litch & 15];
	  }
	  bufptr = buffer + 2 * bytes + 1;
	}
	break;
    case 'K' :  
	while ((ch = get_char(tb)) != EOF && !isspace(ch))
	{
	  if (strchr(PDFIO_DELIM_CHARS, ch) != NULL)
	  {
	    tb->bufptr --;
	    break;
	  }
	  else if (bufptr < bufend)
	  {
	    *bufptr++ = (char)ch;
	  }
	  else
	  {
	    _pdfioFileError(tb->pdf, "Token too large.");
	    return (false);
	  }
	}
	break;
    case 'N' :  
	while ((ch = get_char(tb)) != EOF && !isspace(ch))
	{
	  if (!isdigit(ch) && ch != '.')
	  {
	    tb->bufptr --;
	    break;
	  }
	  else if (bufptr < bufend)
	  {
	    *bufptr++ = (char)ch;
	  }
	  else
	  {
	    _pdfioFileError(tb->pdf, "Token too large.");
	    return (false);
	  }
	}
	break;
    case '/' :  
	while ((ch = get_char(tb)) != EOF && !isspace(ch))
	{
	  if (strchr(PDFIO_DELIM_CHARS, ch) != NULL)
	  {
	    tb->bufptr --;
	    break;
	  }
	  else if (ch == '#')
	  {
	    int	i;			 
	    for (i = 0, ch = 0; i < 2; i ++)
	    {
	      int tch = get_char(tb);
	      if (!isxdigit(tch & 255))
	      {
		_pdfioFileError(tb->pdf, "Bad # escape in name.");
		return (false);
	      }
	      else if (isdigit(tch))
		ch = ((ch & 255) << 4) | (tch - '0');
	      else
		ch = ((ch & 255) << 4) | (tolower(tch) - 'a' + 10);
	    }
	  }
	  if (bufptr < bufend)
	  {
	    *bufptr++ = (char)ch;
	  }
	  else
	  {
	    _pdfioFileError(tb->pdf, "Token too large.");
	    return (false);
	  }
	}
	break;
    case '<' :  
	if ((ch = get_char(tb)) == '<')
	{
	  *bufptr++ = (char)ch;
	  break;
	}
	else if (!isspace(ch & 255) && !isxdigit(ch & 255))
	{
	  _pdfioFileError(tb->pdf, "Syntax error: '<%c'", ch);
	  return (false);
	}
        do
	{
	  if (isxdigit(ch))
	  {
	    if (bufptr < bufend)
	    {
	      *bufptr++ = (char)ch;
	    }
	    else
	    {
	      _pdfioFileError(tb->pdf, "Token too large.");
	      return (false);
	    }
	  }
	  else if (!isspace(ch))
	  {
	    _pdfioFileError(tb->pdf, "Invalid hex string character '%c'.", ch);
	    return (false);
	  }
	}
	while ((ch = get_char(tb)) != EOF && ch != '>');
	if (ch == EOF)
	{
	  _pdfioFileError(tb->pdf, "Unterminated hex string.");
	  return (false);
	}
	break;
    case '>' :  
	if ((ch = get_char(tb)) == '>')
	{
	  *bufptr++ = '>';
	}
	else
	{
	  _pdfioFileError(tb->pdf, "Syntax error: '>%c'.", ch);
	  return (false);
	}
	break;
  }
  *bufptr = '\0';
  PDFIO_DEBUG("_pdfioTokenRead: Read '%s'.\n", buffer);
  return (bufptr > buffer);
}