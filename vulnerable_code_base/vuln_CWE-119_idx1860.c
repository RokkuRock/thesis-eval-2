find_file (const char *currpath, grub_fshelp_node_t currroot,
	   grub_fshelp_node_t *currfound,
	   struct grub_fshelp_find_file_closure *c)
{
#ifndef _MSC_VER
	char fpath[grub_strlen (currpath) + 1];
#else
	char *fpath = grub_malloc (grub_strlen (currpath) + 1);
#endif
  char *name = fpath;
  char *next;
  enum grub_fshelp_filetype type = GRUB_FSHELP_DIR;
  grub_fshelp_node_t currnode = currroot;
  grub_fshelp_node_t oldnode = currroot;
  c->currroot = currroot;
  grub_strncpy (fpath, currpath, grub_strlen (currpath) + 1);
  while (*name == '/')
    name++;
  if (! *name)
    {
      *currfound = currnode;
      return 0;
    }
  for (;;)
    {
      int found;
      struct find_file_closure cc;
      next = grub_strchr (name, '/');
      if (next)
	{
	  while (*next == '/')
	    *(next++) = '\0';
	}
      if (type != GRUB_FSHELP_DIR)
	{
	  free_node (currnode, c);
	  return grub_error (GRUB_ERR_BAD_FILE_TYPE, "not a directory");
	}
      cc.name = name;
      cc.type = &type;
      cc.oldnode = &oldnode;
      cc.currnode = &currnode;
      found = c->iterate_dir (currnode, iterate, &cc);
      if (! found)
	{
	  if (grub_errno)
	    return grub_errno;
	  break;
	}
      if (type == GRUB_FSHELP_SYMLINK)
	{
	  char *symlink;
	  if (++(c->symlinknest) == 8)
	    {
	      free_node (currnode, c);
	      free_node (oldnode, c);
	      return grub_error (GRUB_ERR_SYMLINK_LOOP,
				 "too deep nesting of symlinks");
	    }
	  symlink = c->read_symlink (currnode);
	  free_node (currnode, c);
	  if (!symlink)
	    {
	      free_node (oldnode, c);
	      return grub_errno;
	    }
	  if (symlink[0] == '/')
	    {
	      free_node (oldnode, c);
	      oldnode = c->rootnode;
	    }
	  find_file (symlink, oldnode, &currnode, c);
	  type = c->foundtype;
	  grub_free (symlink);
	  if (grub_errno)
	    {
	      free_node (oldnode, c);
	      return grub_errno;
	    }
	}
      free_node (oldnode, c);
      if (! next || *next == '\0')
	{
	  *currfound = currnode;
	  c->foundtype = type;
	  return 0;
	}
      name = next;
    }
  return grub_error (GRUB_ERR_FILE_NOT_FOUND, "file not found");
}