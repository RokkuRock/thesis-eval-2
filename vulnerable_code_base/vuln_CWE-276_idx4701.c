flatpak_transaction_add_ref (FlatpakTransaction             *self,
                             const char                     *remote,
                             FlatpakDecomposed              *ref,
                             const char                    **subpaths,
                             const char                    **previous_ids,
                             const char                     *commit,
                             FlatpakTransactionOperationType kind,
                             GFile                          *bundle,
                             const char                     *external_metadata,
                             gboolean                        pin_on_deploy,
                             GError                        **error)
{
  FlatpakTransactionPrivate *priv = flatpak_transaction_get_instance_private (self);
  g_autofree char *origin = NULL;
  g_auto(GStrv) new_subpaths = NULL;
  const char *pref;
  g_autofree char *origin_remote = NULL;
  g_autoptr(FlatpakRemoteState) state = NULL;
  FlatpakTransactionOperation *op;
  if (remote_name_is_file (remote))
    {
      gboolean changed_config;
      g_autofree char *id = flatpak_decomposed_dup_id (ref);
      origin_remote = flatpak_dir_create_origin_remote (priv->dir,
                                                        remote,  
                                                        id,
                                                        "Local repo",
                                                        flatpak_decomposed_get_ref (ref),
                                                        NULL,
                                                        NULL,
                                                        &changed_config,
                                                        NULL, error);
      if (origin_remote == NULL)
        return FALSE;
      if (changed_config)
        flatpak_installation_drop_caches (priv->installation, NULL, NULL);
      g_ptr_array_add (priv->added_origin_remotes, g_strdup (origin_remote));
      remote = origin_remote;
    }
  pref = flatpak_decomposed_get_pref (ref);
  if (kind == FLATPAK_TRANSACTION_OPERATION_UPDATE)
    {
      g_autoptr(GBytes) deploy_data = NULL;
      if (!dir_ref_is_installed (priv->dir, ref, &origin, &deploy_data))
        return flatpak_fail_error (error, FLATPAK_ERROR_NOT_INSTALLED,
                                   _("%s not installed"), pref);
      if (flatpak_dir_get_remote_disabled (priv->dir, origin))
        {
          g_debug (_("Remote %s disabled, ignoring %s update"), origin, pref);
          return TRUE;
        }
      remote = origin;
      if (subpaths == NULL)
        {
          g_autofree const char **old_subpaths = flatpak_deploy_data_get_subpaths (deploy_data);
          if (flatpak_decomposed_id_has_suffix (ref, ".Locale"))
            {
              g_auto(GStrv) extra_subpaths = flatpak_dir_get_locale_subpaths (priv->dir);
              new_subpaths = flatpak_subpaths_merge ((char **)old_subpaths, extra_subpaths);
            }
          else
            {
              new_subpaths = g_strdupv ((char **)old_subpaths);
            }
          subpaths = (const char **)new_subpaths;
        }
    }
  else if (kind == FLATPAK_TRANSACTION_OPERATION_INSTALL)
    {
      if (!priv->reinstall &&
          dir_ref_is_installed (priv->dir, ref, &origin, NULL))
        {
          if (g_strcmp0 (remote, origin) == 0)
            return flatpak_fail_error (error, FLATPAK_ERROR_ALREADY_INSTALLED,
                                       _("%s is already installed"), pref);
          else
            return flatpak_fail_error (error, FLATPAK_ERROR_DIFFERENT_REMOTE,
                                       _("%s is already installed from remote %s"),
                                       pref, origin);
        }
    }
  else if (kind == FLATPAK_TRANSACTION_OPERATION_UNINSTALL)
    {
      if (!dir_ref_is_installed (priv->dir, ref, &origin, NULL))
        return flatpak_fail_error (error, FLATPAK_ERROR_NOT_INSTALLED,
                                   _("%s not installed"), pref);
      remote = origin;
    }
  g_assert (remote != NULL);
  if (kind != FLATPAK_TRANSACTION_OPERATION_UNINSTALL)
    {
      g_autofree char *arch = flatpak_decomposed_dup_arch (ref);
      state = flatpak_transaction_ensure_remote_state (self, kind, remote, arch, error);
      if (state == NULL)
        return FALSE;
    }
  op = flatpak_transaction_add_op (self, remote, ref, subpaths, previous_ids,
                                   commit, bundle, kind, pin_on_deploy, error);
  if (op == NULL)
    return FALSE;
  if (external_metadata)
    op->external_metadata = g_bytes_new (external_metadata, strlen (external_metadata) + 1);
  return TRUE;
}