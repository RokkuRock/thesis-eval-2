mark_op_resolved (FlatpakTransactionOperation *op,
                  const char                  *commit,
                  GFile                       *sideload_path,
                  GBytes                      *metadata,
                  GBytes                      *old_metadata)
{
  g_debug ("marking op %s:%s resolved to %s", kind_to_str (op->kind), flatpak_decomposed_get_ref (op->ref), commit ? commit : "-");
  g_assert (op != NULL);
  g_assert (commit != NULL);
  op->resolved = TRUE;
  if (op->resolved_commit != commit)
    {
      g_free (op->resolved_commit);  
      op->resolved_commit = g_strdup (commit);
    }
  if (sideload_path)
    op->resolved_sideload_path = g_object_ref (sideload_path);
  if (metadata)
    {
      g_autoptr(GKeyFile) metakey = g_key_file_new ();
      if (g_key_file_load_from_bytes (metakey, metadata, G_KEY_FILE_NONE, NULL))
        {
          op->resolved_metadata = g_bytes_ref (metadata);
          op->resolved_metakey = g_steal_pointer (&metakey);
        }
      else
        g_message ("Warning: Failed to parse metadata for %s\n", flatpak_decomposed_get_ref (op->ref));
    }
  if (old_metadata)
    {
      g_autoptr(GKeyFile) metakey = g_key_file_new ();
      if (g_key_file_load_from_bytes (metakey, old_metadata, G_KEY_FILE_NONE, NULL))
        {
          op->resolved_old_metadata = g_bytes_ref (old_metadata);
          op->resolved_old_metakey = g_steal_pointer (&metakey);
        }
      else
        g_message ("Warning: Failed to parse old metadata for %s\n", flatpak_decomposed_get_ref (op->ref));
    }
}