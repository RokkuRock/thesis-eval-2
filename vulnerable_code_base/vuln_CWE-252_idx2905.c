_dl_dst_substitute (struct link_map *l, const char *name, char *result,
		    int is_path)
{
  char *last_elem, *wp;
  last_elem = wp = result;
  do
    {
      if (*name == '$')
	{
	  const char *repl;
	  size_t len;
	  if ((((strncmp (&name[1], "ORIGIN", 6) == 0 && (len = 7) != 0)
		|| (strncmp (&name[1], "PLATFORM", 8) == 0 && (len = 9) != 0))
	       && (name[len] == '\0' || name[len] == '/'
		   || (is_path && name[len] == ':')))
	      || (name[1] == '{'
		  && ((strncmp (&name[2], "ORIGIN}", 7) == 0 && (len = 9) != 0)
		      || (strncmp (&name[2], "PLATFORM}", 9) == 0
			  && (len = 11) != 0))))
	    {
	      repl = ((len == 7 || name[2] == 'O')
		      ? (__libc_enable_secure ? NULL : l->l_origin)
		      : _dl_platform);
	      if (repl != NULL && repl != (const char *) -1)
		{
		  wp = __stpcpy (wp, repl);
		  name += len;
		}
	      else
		{
		  wp = last_elem;
		  name += len;
		  while (*name != '\0' && (!is_path || *name != ':'))
		    ++name;
		}
	    }
	  else
	    *wp++ = *name++;
	}
      else if (is_path && *name == ':')
	{
	  *wp++ = *name++;
	  last_elem = wp;
	}
      else
	*wp++ = *name++;
    }
  while (*name != '\0');
  *wp = '\0';
  return result;
}