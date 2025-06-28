flatpak_context_set_persistent (FlatpakContext *context,
                                const char     *path)
{
  g_hash_table_insert (context->persistent, g_strdup (path), GINT_TO_POINTER (1));
}