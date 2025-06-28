htmlGetText(tree_t *t)		 
{
  uchar		*s,		 
		*s2,		 
		*tdata = NULL,	 
		*talloc = NULL;	 
  size_t	slen,		 
		tlen;		 
  slen = 0;
  s    = NULL;
  while (t != NULL)
  {
    if (t->child)
      tdata = talloc = htmlGetText(t->child);
    else
      tdata = t->data;
    if (tdata != NULL)
    {
      tlen = strlen((char *)tdata);
      if (s)
        s2 = (uchar *)realloc(s, 1 + slen + tlen);
      else
        s2 = (uchar *)malloc(1 + tlen);
      if (!s2)
        break;
      s = s2;
      memcpy((char *)s + slen, (char *)tdata, tlen);
      slen += tlen;
      if (talloc)
      {
	free(talloc);
	talloc = NULL;
      }
    }
    t = t->next;
  }
  if (slen)
    s[slen] = '\0';
  if (talloc)
    free(talloc);
  return (s);
}