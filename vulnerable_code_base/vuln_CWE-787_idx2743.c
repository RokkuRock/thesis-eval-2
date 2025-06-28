write_file(tree_t *t,		 
           FILE   *fp,		 
           int    col)		 
{
  int	i;			 
  uchar	*ptr;			 
  while (t != NULL)
  {
    if (t->markup == MARKUP_NONE)
    {
      if (t->preformatted)
      {
        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fputs((char *)iso8859(*ptr), fp);
	if (t->data[strlen((char *)t->data) - 1] == '\n')
          col = 0;
	else
          col += strlen((char *)t->data);
      }
      else
      {
	if ((col + (int)strlen((char *)t->data)) > 72 && col > 0)
	{
          putc('\n', fp);
          col = 0;
	}
        for (ptr = t->data; *ptr != '\0'; ptr ++)
          fputs((char *)iso8859(*ptr), fp);
	col += strlen((char *)t->data);
	if (col > 72)
	{
          putc('\n', fp);
          col = 0;
	}
      }
    }
    else if (t->markup == MARKUP_COMMENT)
      fprintf(fp, "\n<!--%s-->\n", t->data);
    else if (t->markup > 0)
    {
      switch (t->markup)
      {
        case MARKUP_AREA :
        case MARKUP_BR :
        case MARKUP_CENTER :
        case MARKUP_COMMENT :
        case MARKUP_DD :
        case MARKUP_DL :
        case MARKUP_DT :
        case MARKUP_H1 :
        case MARKUP_H2 :
        case MARKUP_H3 :
        case MARKUP_H4 :
        case MARKUP_H5 :
        case MARKUP_H6 :
        case MARKUP_HEAD :
        case MARKUP_HR :
        case MARKUP_LI :
        case MARKUP_MAP :
        case MARKUP_OL :
        case MARKUP_P :
        case MARKUP_PRE :
        case MARKUP_TABLE :
        case MARKUP_TITLE :
        case MARKUP_TR :
        case MARKUP_UL :
	case MARKUP_DIR :
	case MARKUP_MENU :
            if (col > 0)
            {
              putc('\n', fp);
              col = 0;
            }
        default :
            break;
      }
      col += fprintf(fp, "<%s", _htmlMarkups[t->markup]);
      for (i = 0; i < t->nvars; i ++)
      {
	if (col > 72 && !t->preformatted)
	{
          putc('\n', fp);
          col = 0;
	}
        if (col > 0)
        {
          putc(' ', fp);
          col ++;
        }
	if (t->vars[i].value == NULL)
          col += fprintf(fp, "%s", t->vars[i].name);
	else if (strchr((char *)t->vars[i].value, '\"') != NULL)
          col += fprintf(fp, "%s=\'%s\'", t->vars[i].name, t->vars[i].value);
	else
          col += fprintf(fp, "%s=\"%s\"", t->vars[i].name, t->vars[i].value);
      }
      putc('>', fp);
      col ++;
      if (col > 72 && !t->preformatted)
      {
	putc('\n', fp);
	col = 0;
      }
      if (t->child != NULL)
      {
	col = write_file(t->child, fp, col);
	if (col > 72 && !t->preformatted)
	{
	  putc('\n', fp);
	  col = 0;
	}
        col += fprintf(fp, "</%s>", _htmlMarkups[t->markup]);
        switch (t->markup)
        {
          case MARKUP_AREA :
          case MARKUP_BR :
          case MARKUP_CENTER :
          case MARKUP_COMMENT :
          case MARKUP_DD :
          case MARKUP_DL :
          case MARKUP_DT :
          case MARKUP_H1 :
          case MARKUP_H2 :
          case MARKUP_H3 :
          case MARKUP_H4 :
          case MARKUP_H5 :
          case MARKUP_H6 :
          case MARKUP_HEAD :
          case MARKUP_HR :
          case MARKUP_LI :
          case MARKUP_MAP :
          case MARKUP_OL :
          case MARKUP_P :
          case MARKUP_PRE :
          case MARKUP_TABLE :
          case MARKUP_TITLE :
          case MARKUP_TR :
          case MARKUP_UL :
          case MARKUP_DIR :
          case MARKUP_MENU :
              putc('\n', fp);
              col = 0;
          default :
	      break;
        }
      }
    }
    t = t->next;
  }
  return (col);
}