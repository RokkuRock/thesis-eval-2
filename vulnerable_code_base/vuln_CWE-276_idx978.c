try_resolve_op_from_metadata (FlatpakTransaction *self,
                              FlatpakTransactionOperation *op,
                              const char *checksum,
                              GFile *sideload_path,
                              FlatpakRemoteState *state)
{
  g_autoptr(GBytes) metadata_bytes = NULL;
  guint64 download_size = 0;
  guint64 installed_size = 0;
  const char *metadata = NULL;
  VarMetadataRef sparse_cache;
  VarRefInfoRef info;
  g_autofree char *summary_checksum = NULL;
  if ((state->summary == NULL && state->index == NULL) ||
      !flatpak_remote_state_lookup_ref (state, flatpak_decomposed_get_ref (op->ref),
                                        &summary_checksum, NULL, NULL, NULL, NULL) ||
      strcmp (summary_checksum, checksum) != 0)
    return FALSE;
  if (!flatpak_remote_state_lookup_cache (state, flatpak_decomposed_get_ref (op->ref),
                                          &download_size, &installed_size, &metadata, NULL))
      return FALSE;
  metadata_bytes = g_bytes_new (metadata, strlen (metadata));
  if (flatpak_remote_state_lookup_ref (state, flatpak_decomposed_get_ref (op->ref),
                                       NULL, NULL, &info, NULL, NULL))
    op->summary_metadata = var_metadata_dup_to_gvariant (var_ref_info_get_metadata (info));
  op->installed_size = installed_size;
  op->download_size = download_size;
  op->token_type = state->default_token_type;
  if (flatpak_remote_state_lookup_sparse_cache (state, flatpak_decomposed_get_ref (op->ref), &sparse_cache, NULL))
    {
      op->eol = g_strdup (var_metadata_lookup_string (sparse_cache, FLATPAK_SPARSE_CACHE_KEY_ENDOFLINE, NULL));
      op->eol_rebase = g_strdup (var_metadata_lookup_string (sparse_cache, FLATPAK_SPARSE_CACHE_KEY_ENDOFLINE_REBASE, NULL));
      op->token_type = GINT32_FROM_LE (var_metadata_lookup_int32 (sparse_cache, FLATPAK_SPARSE_CACHE_KEY_TOKEN_TYPE, op->token_type));
    }
  resolve_op_end (self, op, checksum, sideload_path, metadata_bytes);
  return TRUE;
}