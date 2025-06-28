_dl_dst_count (const char *name, int is_path)
{
  size_t cnt = 0;
  do
    {
      size_t len = 1;
      if ((((!__libc_enable_secure
	     && strncmp (&name[1], "ORIGIN", 6) == 0 && (len = 7) != 0)
	    || (strncmp (&name[1], "PLATFORM", 8) == 0 && (len = 9) != 0))
	   && (name[len] == '\0' || name[len] == '/'
	       || (is_path && name[len] == ':')))
	  || (name[1] == '{'
	      && ((!__libc_enable_secure
		   && strncmp (&name[2], "ORIGIN}", 7) == 0 && (len = 9) != 0)
		  || (strncmp (&name[2], "PLATFORM}", 9) == 0
		      && (len = 11) != 0))))
	++cnt;
      name = strchr (name + len, '$');
    }
  while (name != NULL);
  return cnt;
}