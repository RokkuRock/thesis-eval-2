glob_vector (pat, dir, flags)
     char *pat;
     char *dir;
     int flags;
{
  DIR *d;
  register struct dirent *dp;
  struct globval *lastlink, *e, *dirlist;
  register struct globval *nextlink;
  register char *nextname, *npat, *subdir;
  unsigned int count;
  int lose, skip, ndirs, isdir, sdlen, add_current, patlen;
  register char **name_vector;
  register unsigned int i;
  int mflags;		 
  int pflags;		 
  int nalloca;
  struct globval *firstmalloc, *tmplink;
  char *convfn;
  lastlink = 0;
  count = lose = skip = add_current = 0;
  firstmalloc = 0;
  nalloca = 0;
  name_vector = NULL;
  if (pat == 0 || *pat == '\0')
    {
      if (glob_testdir (dir, 0) < 0)
	return ((char **) &glob_error_return);
      nextlink = (struct globval *)alloca (sizeof (struct globval));
      if (nextlink == NULL)
	return ((char **) NULL);
      nextlink->next = (struct globval *)0;
      nextname = (char *) malloc (1);
      if (nextname == 0)
	lose = 1;
      else
	{
	  lastlink = nextlink;
	  nextlink->name = nextname;
	  nextname[0] = '\0';
	  count = 1;
	}
      skip = 1;
    }
  patlen = (pat && *pat) ? strlen (pat) : 0;
  if (skip == 0 && glob_pattern_p (pat) == 0)
    {
      int dirlen;
      struct stat finfo;
      if (glob_testdir (dir, 0) < 0)
	return ((char **) &glob_error_return);
      dirlen = strlen (dir);
      nextname = (char *)malloc (dirlen + patlen + 2);
      npat = (char *)malloc (patlen + 1);
      if (nextname == 0 || npat == 0)
	{
	  FREE (nextname);
	  FREE (npat);
	  lose = 1;
	}
      else
	{
	  strcpy (npat, pat);
	  dequote_pathname (npat);
	  strcpy (nextname, dir);
	  nextname[dirlen++] = '/';
	  strcpy (nextname + dirlen, npat);
	  if (GLOB_TESTNAME (nextname) >= 0)
	    {
	      free (nextname);
	      nextlink = (struct globval *)alloca (sizeof (struct globval));
	      if (nextlink)
		{
		  nextlink->next = (struct globval *)0;
		  lastlink = nextlink;
		  nextlink->name = npat;
		  count = 1;
		}
	      else
		{
		  free (npat);
		  lose = 1;
		}
	    }
	  else
	    {
	      free (nextname);
	      free (npat);
	    }
	}
      skip = 1;
    }
  if (skip == 0)
    {
#if defined (OPENDIR_NOT_ROBUST)
      if (glob_testdir (dir, 0) < 0)
	return ((char **) &glob_error_return);
#endif
      d = opendir (dir);
      if (d == NULL)
	return ((char **) &glob_error_return);
      mflags = (noglob_dot_filenames ? FNM_PERIOD : 0) | FNM_PATHNAME;
#ifdef FNM_CASEFOLD
      if (glob_ignore_case)
	mflags |= FNM_CASEFOLD;
#endif
      if (extended_glob)
	mflags |= FNM_EXTMATCH;
      add_current = ((flags & (GX_ALLDIRS|GX_ADDCURDIR)) == (GX_ALLDIRS|GX_ADDCURDIR));
      while (1)
	{
	  if (interrupt_state || terminating_signal)
	    {
	      lose = 1;
	      break;
	    }
	  else if (signal_is_pending (SIGINT))	 
	    {
	      lose = 1;
	      break;
	    }
	  dp = readdir (d);
	  if (dp == NULL)
	    break;
	  if (REAL_DIR_ENTRY (dp) == 0)
	    continue;
#if 0
	  if (dp->d_name == 0 || *dp->d_name == 0)
	    continue;
#endif
#if HANDLE_MULTIBYTE
	  if (MB_CUR_MAX > 1 && mbskipname (pat, dp->d_name, flags))
	    continue;
	  else
#endif
	  if (skipname (pat, dp->d_name, flags))
	    continue;
	  if (flags & (GX_MATCHDIRS|GX_ALLDIRS))
	    {
	      pflags = (flags & GX_ALLDIRS) ? MP_RMDOT : 0;
	      if (flags & GX_NULLDIR)
		pflags |= MP_IGNDOT;
	      subdir = sh_makepath (dir, dp->d_name, pflags);
	      isdir = glob_testdir (subdir, flags);
	      if (isdir < 0 && (flags & GX_MATCHDIRS))
		{
		  free (subdir);
		  continue;
		}
	    }
	  if (flags & GX_ALLDIRS)
	    {
	      if (isdir == 0)
		{
		  dirlist = finddirs (pat, subdir, (flags & ~GX_ADDCURDIR), &e, &ndirs);
		  if (dirlist == &finddirs_error_return)
		    {
		      free (subdir);
		      lose = 1;
		      break;
		    }
		  if (ndirs)		 
		    {
		      if (firstmalloc == 0)
		        firstmalloc = e;
		      e->next = lastlink;
		      lastlink = dirlist;
		      count += ndirs;
		    }
		}
	      nextlink = (struct globval *) malloc (sizeof (struct globval));
	      if (firstmalloc == 0)
		firstmalloc = nextlink;
	      sdlen = strlen (subdir);
	      nextname = (char *) malloc (sdlen + 1);
	      if (nextlink == 0 || nextname == 0)
		{
		  FREE (nextlink);
		  FREE (nextname);
		  free (subdir);
		  lose = 1;
		  break;
		}
	      nextlink->next = lastlink;
	      lastlink = nextlink;
	      nextlink->name = nextname;
	      bcopy (subdir, nextname, sdlen + 1);
	      free (subdir);
	      ++count;
	      continue;
	    }
	  else if (flags & GX_MATCHDIRS)
	    free (subdir);
	  convfn = fnx_fromfs (dp->d_name, D_NAMLEN (dp));
	  if (strmatch (pat, convfn, mflags) != FNM_NOMATCH)
	    {
	      if (nalloca < ALLOCA_MAX)
		{
		  nextlink = (struct globval *) alloca (sizeof (struct globval));
		  nalloca += sizeof (struct globval);
		}
	      else
		{
		  nextlink = (struct globval *) malloc (sizeof (struct globval));
		  if (firstmalloc == 0)
		    firstmalloc = nextlink;
		}
	      nextname = (char *) malloc (D_NAMLEN (dp) + 1);
	      if (nextlink == 0 || nextname == 0)
		{
		  FREE (nextlink);
		  FREE (nextname);
		  lose = 1;
		  break;
		}
	      nextlink->next = lastlink;
	      lastlink = nextlink;
	      nextlink->name = nextname;
	      bcopy (dp->d_name, nextname, D_NAMLEN (dp) + 1);
	      ++count;
	    }
	}
      (void) closedir (d);
    }
  if (add_current)
    {
      sdlen = strlen (dir);
      nextname = (char *)malloc (sdlen + 1);
      nextlink = (struct globval *) malloc (sizeof (struct globval));
      if (nextlink == 0 || nextname == 0)
	{
	  FREE (nextlink);
	  FREE (nextname);
	  lose = 1;
	}
      else
	{
	  nextlink->name = nextname;
	  nextlink->next = lastlink;
	  lastlink = nextlink;
	  if (flags & GX_NULLDIR)
	    nextname[0] = '\0';
	  else
	    bcopy (dir, nextname, sdlen + 1);
	  ++count;
	}
    }
  if (lose == 0)
    {
      name_vector = (char **) malloc ((count + 1) * sizeof (char *));
      lose |= name_vector == NULL;
    }
  if (lose)
    {
      tmplink = 0;
      while (lastlink)
	{
	  if (firstmalloc)
	    {
	      if (lastlink == firstmalloc)
		firstmalloc = 0;
	      tmplink = lastlink;
	    }
	  else
	    tmplink = 0;
	  free (lastlink->name);
	  lastlink = lastlink->next;
	  FREE (tmplink);
	}
      return ((char **)NULL);
    }
  for (tmplink = lastlink, i = 0; i < count; ++i)
    {
      name_vector[i] = tmplink->name;
      tmplink = tmplink->next;
    }
  name_vector[count] = NULL;
  if (firstmalloc)
    {
      tmplink = 0;
      while (lastlink)
	{
	  tmplink = lastlink;
	  if (lastlink == firstmalloc)
	    lastlink = firstmalloc = 0;
	  else
	    lastlink = lastlink->next;
	  free (tmplink);
	}
    }
  return (name_vector);
}