write_node(FILE   *out,		 
           tree_t *t,		 
           int    col)		 
{
  int		i;		 
  uchar		*ptr,		 
		*entity,	 
		*src,		 
		*realsrc,	 
		newsrc[1024];	 
  if (out == NULL)
    return (0);
  switch (t->markup)
  {
    case MARKUP_NONE :
        if (t->data == NULL)
	  break;
	if (t->preformatted)
	{
          for (ptr = t->data; *ptr; ptr ++)
            fputs((char *)iso8859(*ptr), out);
	  if (t->data[strlen((char *)t->data) - 1] == '\n')
            col = 0;
	  else
            col += strlen((char *)t->data);
	}
	else
	{
	  if ((col + (int)strlen((char *)t->data)) > 72 && col > 0)
	  {
            putc('\n', out);
            col = 0;
	  }
          for (ptr = t->data; *ptr; ptr ++)
            fputs((char *)iso8859(*ptr), out);
	  col += strlen((char *)t->data);
	  if (col > 72)
	  {
            putc('\n', out);
            col = 0;
	  }
	}
	break;
    case MARKUP_COMMENT :
    case MARKUP_UNKNOWN :
        fputs("\n<!--", out);
	for (ptr = t->data; *ptr; ptr ++)
	  fputs((char *)iso8859(*ptr), out);
	fputs("-->\n", out);
	col = 0;
	break;
    case MARKUP_AREA :
    case MARKUP_BODY :
    case MARKUP_DOCTYPE :
    case MARKUP_ERROR :
    case MARKUP_FILE :
    case MARKUP_HEAD :
    case MARKUP_HTML :
    case MARKUP_MAP :
    case MARKUP_META :
    case MARKUP_TITLE :
        break;
    case MARKUP_BR :
    case MARKUP_CENTER :
    case MARKUP_DD :
    case MARKUP_DL :
    case MARKUP_DT :
    case MARKUP_H1 :
    case MARKUP_H2 :
    case MARKUP_H3 :
    case MARKUP_H4 :
    case MARKUP_H5 :
    case MARKUP_H6 :
    case MARKUP_H7 :
    case MARKUP_H8 :
    case MARKUP_H9 :
    case MARKUP_H10 :
    case MARKUP_H11 :
    case MARKUP_H12 :
    case MARKUP_H13 :
    case MARKUP_H14 :
    case MARKUP_H15 :
    case MARKUP_HR :
    case MARKUP_LI :
    case MARKUP_OL :
    case MARKUP_P :
    case MARKUP_PRE :
    case MARKUP_TABLE :
    case MARKUP_TR :
    case MARKUP_UL :
        if (col > 0)
        {
          putc('\n', out);
          col = 0;
        }
    default :
	if (t->markup == MARKUP_IMG &&
            (src = htmlGetVariable(t, (uchar *)"SRC")) != NULL &&
            (realsrc = htmlGetVariable(t, (uchar *)"REALSRC")) != NULL)
	{
          if (file_method((char *)src) == NULL &&
              src[0] != '/' && src[0] != '\\' &&
	      (!isalpha(src[0]) || src[1] != ':'))
          {
            image_copy((char *)src, (char *)realsrc, OutputPath);
            strlcpy((char *)newsrc, file_basename((char *)src), sizeof(newsrc));
            htmlSetVariable(t, (uchar *)"SRC", newsrc);
          }
	}
        if (t->markup != MARKUP_EMBED)
	{
	  col += fprintf(out, "<%s", _htmlMarkups[t->markup]);
	  for (i = 0; i < t->nvars; i ++)
	  {
	    if (strcasecmp((char *)t->vars[i].name, "BREAK") == 0 &&
	        t->markup == MARKUP_HR)
	      continue;
	    if (strcasecmp((char *)t->vars[i].name, "REALSRC") == 0 &&
	        t->markup == MARKUP_IMG)
	      continue;
            if (strncasecmp((char *)t->vars[i].name, "_HD_", 4) == 0)
	      continue;
	    if (col > 72 && !t->preformatted)
	    {
              putc('\n', out);
              col = 0;
	    }
            if (col > 0)
            {
              putc(' ', out);
              col ++;
            }
	    if (t->vars[i].value == NULL)
              col += fprintf(out, "%s", t->vars[i].name);
	    else
	    {
	      col += fprintf(out, "%s=\"", t->vars[i].name);
	      for (ptr = t->vars[i].value; *ptr; ptr ++)
	      {
		entity = iso8859(*ptr);
		fputs((char *)entity, out);
		col += strlen((char *)entity);
	      }
	      putc('\"', out);
	      col ++;
	    }
	  }
	  putc('>', out);
	  col ++;
	  if (col > 72 && !t->preformatted)
	  {
	    putc('\n', out);
	    col = 0;
	  }
	}
	break;
  }
  return (col);
}