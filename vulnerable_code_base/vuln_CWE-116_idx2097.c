load_kernel_module_list (void)
{
  GHashTable *modules = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  g_autofree char *modules_data = NULL;
  g_autoptr(GError) error = NULL;
  char *start, *end;
  if (!g_file_get_contents ("/proc/modules", &modules_data, NULL, &error))
    {
      g_info ("Failed to read /proc/modules: %s", error->message);
      return modules;
    }
  start = modules_data;
  while (TRUE)
    {
      end = strchr (start, ' ');
      if (end == NULL)
        break;
      g_hash_table_add (modules, g_strndup (start, (end - start)));
      start = strchr (end, '\n');
      if (start == NULL)
        break;
      start++;
    }
  return modules;
}