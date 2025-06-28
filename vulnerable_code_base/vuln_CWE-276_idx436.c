upgrade_deploy_data (GBytes             *deploy_data,
                     GFile              *deploy_dir,
                     FlatpakDecomposed  *ref,
                     OstreeRepo         *repo,
                     GCancellable       *cancellable,
                     GError            **error)
{
  VarDeployDataRef deploy_ref = var_deploy_data_from_bytes (deploy_data);
  g_autoptr(GVariant) metadata = g_variant_ref_sink (var_metadata_peek_as_gvariant (var_deploy_data_get_metadata (deploy_ref)));
  g_auto(GVariantDict) metadata_dict = FLATPAK_VARIANT_DICT_INITIALIZER;
  g_autofree const char **subpaths = NULL;
  g_autoptr(GVariant) res = NULL;
  int i, n, old_version;
  g_variant_dict_init (&metadata_dict, NULL);
  g_variant_dict_insert_value (&metadata_dict, "deploy-version",
                               g_variant_new_int32 (FLATPAK_DEPLOY_VERSION_CURRENT));
  n = g_variant_n_children (metadata);
  for (i = 0; i < n; i++)
    {
      const char *key;
      g_autoptr(GVariant) value = NULL;
      g_variant_get_child (metadata, i, "{&s@v}", &key, &value);
      if (strcmp (key, "deploy-version") == 0)
        continue;
      g_variant_dict_insert_value (&metadata_dict, key, value);
    }
  old_version = flatpak_deploy_data_get_version (deploy_data);
  if (old_version < 1)
    {
      g_autofree char *id = flatpak_decomposed_dup_id (ref);
      add_appdata_to_deploy_data (&metadata_dict, deploy_dir, id);
    }
  if (old_version < 3)
    {
      g_variant_dict_insert_value (&metadata_dict, "timestamp",
                                   g_variant_new_uint64 (0));
    }
  if (old_version < 4)
    {
      const char *commit;
      g_autoptr(GVariant) commit_data = NULL;
      g_autoptr(GVariant) commit_metadata = NULL;
      g_autoptr(GKeyFile) keyfile = NULL;
      g_autoptr(GFile) metadata_file = NULL;
      g_autofree char *metadata_contents = NULL;
      g_autofree char *id = flatpak_decomposed_dup_id (ref);
      commit = flatpak_deploy_data_get_commit (deploy_data);
      if (!ostree_repo_load_commit (repo, commit, &commit_data, NULL, error))
        return NULL;
      commit_metadata = g_variant_get_child_value (commit_data, 0);
      add_commit_metadata_to_deploy_data (&metadata_dict, commit_metadata);
      keyfile = g_key_file_new ();
      metadata_file = g_file_resolve_relative_path (deploy_dir, "metadata");
      if (!g_file_load_contents (metadata_file, cancellable,
                                 &metadata_contents, NULL, NULL, error))
        return NULL;
      if (!g_key_file_load_from_data (keyfile, metadata_contents, -1, 0, error))
        return NULL;
      add_metadata_to_deploy_data (&metadata_dict, keyfile);
      if (old_version >= 1)
        add_appdata_to_deploy_data (&metadata_dict, deploy_dir, id);
    }
  subpaths = flatpak_deploy_data_get_subpaths (deploy_data);
  res = g_variant_ref_sink (g_variant_new ("(ss^ast@a{sv})",
                                           flatpak_deploy_data_get_origin (deploy_data),
                                           flatpak_deploy_data_get_commit (deploy_data),
                                           subpaths,
                                           GUINT64_TO_BE (flatpak_deploy_data_get_installed_size (deploy_data)),
                                           g_variant_dict_end (&metadata_dict)));
  return g_variant_get_data_as_bytes (res);
}