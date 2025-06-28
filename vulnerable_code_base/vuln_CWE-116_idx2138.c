context_parse_args (FlatpakContext *context,
                    ...)
{
  g_autoptr(GError) local_error = NULL;
  g_autoptr(GOptionContext) oc = NULL;
  g_autoptr(GOptionGroup) group = NULL;
  g_autoptr(GPtrArray) args = g_ptr_array_new_with_free_func (g_free);
  g_auto(GStrv) argv = NULL;
  const char *arg;
  va_list ap;
  g_ptr_array_add (args, g_strdup ("argv[0]"));
  va_start (ap, context);
  while ((arg = va_arg (ap, const char *)) != NULL)
    g_ptr_array_add (args, g_strdup (arg));
  va_end (ap);
  g_ptr_array_add (args, NULL);
  argv = (GStrv) g_ptr_array_free (g_steal_pointer (&args), FALSE);
  oc = g_option_context_new ("");
  group = flatpak_context_get_options (context);
  g_option_context_add_group (oc, group);
  g_option_context_parse_strv (oc, &argv, &local_error);
  g_assert_no_error (local_error);
}