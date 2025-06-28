unquoted_glob_pattern_p (string)
     register char *string;
{
  register int c;
  char *send;
  int open, bsquote;
  DECLARE_MBSTATE;
  open = bsquote = 0;
  send = string + strlen (string);
  while (c = *string++)
    {
      switch (c)
	{
	case '?':
	case '*':
	  return (1);
	case '[':
	  open++;
	  continue;
	case ']':
	  if (open)
	    return (1);
	  continue;
	case '+':
	case '@':
	case '!':
	  if (*string == '(')	 
	    return (1);
	  continue;
	case '\\':
	  if (*string != '\0' && *string != '/')
	    {
	      bsquote = 1;
	      string++;
	      continue;
	    }
	  else if (*string == 0)
	    return (0);
	case CTLESC:
	  if (*string++ == '\0')
	    return (0);
	}
#ifdef HANDLE_MULTIBYTE
      string--;
      ADVANCE_CHAR_P (string, send - string);
      string++;
#else
      ADVANCE_CHAR_P (string, send - string);
#endif
    }
  return ((bsquote && posix_glob_backslash) ? 2 : 0);
}