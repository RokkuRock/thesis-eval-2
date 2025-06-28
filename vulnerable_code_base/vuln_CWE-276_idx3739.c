flatpak_pull_from_bundle (OstreeRepo   *repo,
                          GFile        *file,
                          const char   *remote,
                          const char   *ref,
                          gboolean      require_gpg_signature,
                          GCancellable *cancellable,
                          GError      **error)
{
  g_autofree char *metadata_contents = NULL;
  g_autofree char *to_checksum = NULL;
  g_autoptr(GFile) root = NULL;
  g_autoptr(GFile) metadata_file = NULL;
  g_autoptr(GInputStream) in = NULL;
  g_autoptr(OstreeGpgVerifyResult) gpg_result = NULL;
  g_autoptr(GError) my_error = NULL;
  g_autoptr(GVariant) metadata = NULL;
  gboolean metadata_valid;
  g_autofree char *remote_collection_id = NULL;
  g_autofree char *collection_id = NULL;
  metadata = flatpak_bundle_load (file, &to_checksum, NULL, NULL, NULL, &metadata_contents, NULL, NULL, &collection_id, error);
  if (metadata == NULL)
    return FALSE;
  if (!ostree_repo_get_remote_option (repo, remote, "collection-id", NULL,
                                      &remote_collection_id, NULL))
    remote_collection_id = NULL;
  if (remote_collection_id != NULL && collection_id != NULL &&
      strcmp (remote_collection_id, collection_id) != 0)
    return flatpak_fail_error (error, FLATPAK_ERROR_INVALID_DATA, _("Collection ‘%s’ of bundle doesn’t match collection ‘%s’ of remote"),
                               collection_id, remote_collection_id);
  if (!ostree_repo_prepare_transaction (repo, NULL, cancellable, error))
    return FALSE;
  ostree_repo_transaction_set_ref (repo, remote, ref, to_checksum);
  if (!ostree_repo_static_delta_execute_offline (repo,
                                                 file,
                                                 FALSE,
                                                 cancellable,
                                                 error))
    return FALSE;
  gpg_result = ostree_repo_verify_commit_ext (repo, to_checksum,
                                              NULL, NULL, cancellable, &my_error);
  if (gpg_result == NULL)
    {
      if (g_error_matches (my_error, OSTREE_GPG_ERROR, OSTREE_GPG_ERROR_NO_SIGNATURE) &&
          !require_gpg_signature)
        {
          g_clear_error (&my_error);
        }
      else
        {
          g_propagate_error (error, g_steal_pointer (&my_error));
          return FALSE;
        }
    }
  else
    {
      if (ostree_gpg_verify_result_count_valid (gpg_result) == 0  &&
          require_gpg_signature)
        return flatpak_fail_error (error, FLATPAK_ERROR_UNTRUSTED, _("GPG signatures found, but none are in trusted keyring"));
    }
  if (!ostree_repo_read_commit (repo, to_checksum, &root, NULL, NULL, error))
    return FALSE;
  if (!ostree_repo_commit_transaction (repo, NULL, cancellable, error))
    return FALSE;
  metadata_file = g_file_resolve_relative_path (root, "metadata");
  in = (GInputStream *) g_file_read (metadata_file, cancellable, NULL);
  if (in != NULL)
    {
      g_autoptr(GMemoryOutputStream) data_stream = (GMemoryOutputStream *) g_memory_output_stream_new_resizable ();
      if (g_output_stream_splice (G_OUTPUT_STREAM (data_stream), in,
                                  G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE,
                                  cancellable, error) < 0)
        return FALSE;
      g_output_stream_write (G_OUTPUT_STREAM (data_stream), "\0", 1, NULL, NULL);
      metadata_valid =
        metadata_contents != NULL &&
        strcmp (metadata_contents, g_memory_output_stream_get_data (data_stream)) == 0;
    }
  else
    {
      metadata_valid = (metadata_contents == NULL);
    }
  if (!metadata_valid)
    {
      ostree_repo_set_ref_immediate (repo, remote, ref, NULL, cancellable, error);
      return flatpak_fail_error (error, FLATPAK_ERROR_INVALID_DATA, _("Metadata in header and app are inconsistent"));
    }
  return TRUE;
}