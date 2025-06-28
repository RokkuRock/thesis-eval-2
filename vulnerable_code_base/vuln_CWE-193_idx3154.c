char *url_canonize2(char *d, char const * const s, size_t n,
		    unsigned syn33,
		    unsigned m32, unsigned m64, unsigned m96)
{
  size_t i = 0;
  if (d == s)
    for (;s[i] && i < n; d++, i++)
      if (s[i] == '%')
	break;
  for (;s[i] && i < n; d++, i++) {
    unsigned char c = s[i], h1, h2;
    if (c != '%') {
      if (!IS_SYN33(syn33, c) && IS_EXCLUDED(c, m32, m64, m96))
	return NULL;
      *d = c;
      continue;
    }
    if (i >= strlen(s) - 1) return NULL;
    h1 = s[i + 1], h2 = s[i + 2];
    if (!IS_HEX(h1) || !IS_HEX(h2)) {
      *d = '\0';
      return NULL;
    }
#define UNHEX(a) (a - (a >= 'a' ? 'a' - 10 : (a >= 'A' ? 'A' - 10 : '0')))
    c = (UNHEX(h1) << 4) | UNHEX(h2);
    if (!IS_EXCLUDED(c, m32, m64, m96)) {
      *d = c, i += 2;
      continue;
    }
    if (h1 >= 'a'  )
      h1 = h1 - 'a' + 'A';
    if (h2 >= 'a'  )
      h2 = h2 - 'a' + 'A';
    d[0] = '%', d[1] = h1, d[2] = h2;
    d +=2, i += 2;
#undef    UNHEX
  }
  *d = '\0';
  return d;
}