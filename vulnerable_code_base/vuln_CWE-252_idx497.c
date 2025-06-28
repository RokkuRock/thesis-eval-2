expand_dynamic_string_token (struct link_map *l, const char *s)
{
  size_t cnt;
  size_t total;
  char *result;
  cnt = DL_DST_COUNT (s, 1);
  if (cnt == 0)
    return local_strdup (s);
  total = DL_DST_REQUIRED (l, s, strlen (s), cnt);
  result = (char *) malloc (total + 1);
  if (result == NULL)
    return NULL;
  return DL_DST_SUBSTITUTE (l, s, result, 1);
}