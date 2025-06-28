validate_commit_metadata (GVariant   *commit_data,
                          const char *ref,
                          const char *required_metadata,
                          gsize       required_metadata_size,
                          gboolean   require_xa_metadata,
                          GError   **error)
{
  g_autoptr(GVariant) commit_metadata = NULL;
  g_autoptr(GVariant) xa_metadata_v = NULL;
  const char *xa_metadata = NULL;
  gsize xa_metadata_size = 0;
  commit_metadata = g_variant_get_child_value (commit_data, 0);
  if (commit_metadata != NULL)
    {
      xa_metadata_v = g_variant_lookup_value (commit_metadata,
                                              "xa.metadata",
                                              G_VARIANT_TYPE_STRING);
      if (xa_metadata_v)
        xa_metadata = g_variant_get_string (xa_metadata_v, &xa_metadata_size);
    }
  if ((xa_metadata == NULL && require_xa_metadata) ||
      (xa_metadata != NULL && (xa_metadata_size != required_metadata_size ||
                               memcmp (xa_metadata, required_metadata, xa_metadata_size) != 0)))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                   _("Commit metadata for %s not matching expected metadata"), ref);
      return FALSE;
    }
  return TRUE;
}