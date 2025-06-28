option_persist_cb (const gchar *option_name,
                   const gchar *value,
                   gpointer     data,
                   GError     **error)
{
  FlatpakContext *context = data;
  flatpak_context_set_persistent (context, value);
  return TRUE;
}