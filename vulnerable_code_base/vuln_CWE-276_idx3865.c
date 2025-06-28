load_deployed_metadata (FlatpakTransaction *self, FlatpakDecomposed *ref, char **out_commit, char **out_remote)
{
  FlatpakTransactionPrivate *priv = flatpak_transaction_get_instance_private (self);
  g_autoptr(GFile) deploy_dir = NULL;
  g_autoptr(GFile) metadata_file = NULL;
  g_autofree char *metadata_contents = NULL;
  gsize metadata_contents_length;
  deploy_dir = flatpak_dir_get_if_deployed (priv->dir, ref, NULL, NULL);
  if (deploy_dir == NULL)
    return NULL;
  if (out_commit || out_remote)
    {
      g_autoptr(GBytes) deploy_data = NULL;
      deploy_data = flatpak_load_deploy_data (deploy_dir, ref,
                                              flatpak_dir_get_repo (priv->dir),
                                              FLATPAK_DEPLOY_VERSION_ANY, NULL, NULL);
      if (deploy_data == NULL)
        return NULL;
      if (out_commit)
        *out_commit = g_strdup (flatpak_deploy_data_get_commit (deploy_data));
      if (out_remote)
        *out_remote = g_strdup (flatpak_deploy_data_get_origin (deploy_data));
    }
  metadata_file = g_file_get_child (deploy_dir, "metadata");
  if (!g_file_load_contents (metadata_file, NULL, &metadata_contents, &metadata_contents_length, NULL, NULL))
    {
      g_debug ("No metadata in local deploy of %s", flatpak_decomposed_get_ref (ref));
      return NULL;
    }
  return g_bytes_new_take (g_steal_pointer (&metadata_contents), metadata_contents_length + 1);
}