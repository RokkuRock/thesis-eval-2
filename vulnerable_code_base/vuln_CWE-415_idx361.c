file_rlookup(const char *filename)	 
{
  int		i;			 
  cache_t	*wc;			 
  for (i = web_files, wc = web_cache; i > 0; i --, wc ++)
    if (!strcmp(wc->name, filename))
      return (wc->url);
  return (filename);
}