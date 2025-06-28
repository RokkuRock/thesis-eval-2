validate_commit_metadata (GVariant   *commit_data,
                          const char *ref,
                          const char *required_metadata,
                          gboolean   require_xa_metadata,
                          GError   **error)
{
  g_autoptr(GVariant) commit_metadata = NULL;
  const char *xa_metadata = NULL;
  commit_metadata = g_variant_get_child_value (commit_data, 0);
  if (commit_metadata != NULL)
    g_variant_lookup (commit_metadata, "xa.metadata", "&s", &xa_metadata);
  if ((xa_metadata == NULL && require_xa_metadata) ||
      (xa_metadata != NULL && g_strcmp0 (required_metadata, xa_metadata) != 0))
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                   _("Commit metadata for %s not matching expected metadata"), ref);
      return FALSE;
    }
  return TRUE;
}