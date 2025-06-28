static void _imap_quote_string (char *dest, size_t dlen, const char *src,
                                const char *to_quote)
{
  char *pt;
  const char *s;
  pt = dest;
  s  = src;
  *pt++ = '"';
  dlen -= 2;
  for (; *s && dlen; s++)
  {
    if (strchr (to_quote, *s))
    {
      dlen -= 2;
      if (!dlen)
	break;
      *pt++ = '\\';
      *pt++ = *s;
    }
    else
    {
      *pt++ = *s;
      dlen--;
    }
  }
  *pt++ = '"';
  *pt = 0;
}