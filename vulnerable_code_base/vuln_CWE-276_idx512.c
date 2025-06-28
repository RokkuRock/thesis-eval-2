resolve_op_from_commit (FlatpakTransaction *self,
                        FlatpakTransactionOperation *op,
                        const char *checksum,
                        GFile *sideload_path,
                        GVariant *commit_data)
{
  g_autoptr(GBytes) metadata_bytes = NULL;
  g_autoptr(GVariant) commit_metadata = NULL;
  const char *xa_metadata = NULL;
  guint64 download_size = 0;
  guint64 installed_size = 0;
  commit_metadata = g_variant_get_child_value (commit_data, 0);
  g_variant_lookup (commit_metadata, "xa.metadata", "&s", &xa_metadata);
  if (xa_metadata == NULL)
    g_message ("Warning: No xa.metadata in local commit %s ref %s", checksum, flatpak_decomposed_get_ref (op->ref));
  else
    metadata_bytes = g_bytes_new (xa_metadata, strlen (xa_metadata) + 1);
  if (g_variant_lookup (commit_metadata, "xa.download-size", "t", &download_size))
    op->download_size = GUINT64_FROM_BE (download_size);
  if (g_variant_lookup (commit_metadata, "xa.installed-size", "t", &installed_size))
    op->installed_size = GUINT64_FROM_BE (installed_size);
  g_variant_lookup (commit_metadata, OSTREE_COMMIT_META_KEY_ENDOFLIFE, "s", &op->eol);
  g_variant_lookup (commit_metadata, OSTREE_COMMIT_META_KEY_ENDOFLIFE_REBASE, "s", &op->eol_rebase);
  resolve_op_end (self, op, checksum, sideload_path, metadata_bytes);
}