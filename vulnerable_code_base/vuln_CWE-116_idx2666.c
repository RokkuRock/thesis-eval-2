print_perm_line (int        idx,
                 GPtrArray *items,
                 int        cols)
{
  g_autoptr(GString) res = g_string_new (NULL);
  int i;
  g_string_append_printf (res, "    [%d] %s", idx, (char *) items->pdata[0]);
  for (i = 1; i < items->len; i++)
    {
      char *p;
      int len;
      p = strrchr (res->str, '\n');
      if (!p)
        p = res->str;
      len = (res->str + strlen (res->str)) - p;
      if (len + strlen ((char *) items->pdata[i]) + 2 >= cols)
        g_string_append_printf (res, ",\n        %s", (char *) items->pdata[i]);
      else
        g_string_append_printf (res, ", %s", (char *) items->pdata[i]);
    }
  g_print ("%s\n", res->str);
}