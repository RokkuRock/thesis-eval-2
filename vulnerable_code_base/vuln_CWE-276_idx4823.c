resolve_ops (FlatpakTransaction *self,
             GCancellable       *cancellable,
             GError            **error)
{
  FlatpakTransactionPrivate *priv = flatpak_transaction_get_instance_private (self);
  GList *l;
  for (l = priv->ops; l != NULL; l = l->next)
    {
      FlatpakTransactionOperation *op = l->data;
      g_autoptr(FlatpakRemoteState) state = NULL;
      g_autofree char *checksum = NULL;
      g_autoptr(GBytes) metadata_bytes = NULL;
      if (op->resolved)
        continue;
      if (op->skip)
        {
          g_assert (op->resolved_commit != NULL);
          mark_op_resolved (op, op->resolved_commit, NULL, NULL, NULL);
          continue;
        }
      if (op->kind == FLATPAK_TRANSACTION_OPERATION_UNINSTALL)
        {
          metadata_bytes = load_deployed_metadata (self, op->ref, &checksum, NULL);
          if (metadata_bytes == NULL)
            {
              op->skip = TRUE;
              continue;
            }
          mark_op_resolved (op, checksum, NULL, metadata_bytes, NULL);
          continue;
        }
      if (op->kind == FLATPAK_TRANSACTION_OPERATION_INSTALL_BUNDLE)
        {
          g_assert (op->commit != NULL);
          mark_op_resolved (op, op->commit, NULL, op->external_metadata, NULL);
          continue;
        }
      if (flatpak_decomposed_is_app (op->ref))
        {
          if (op->kind == FLATPAK_TRANSACTION_OPERATION_INSTALL)
            priv->max_op = APP_INSTALL;
          else
            priv->max_op = MAX (priv->max_op, APP_UPDATE);
        }
      else if (flatpak_decomposed_is_runtime (op->ref))
        {
          if (op->kind == FLATPAK_TRANSACTION_OPERATION_INSTALL)
            priv->max_op = MAX (priv->max_op, RUNTIME_INSTALL);
        }
      state = flatpak_transaction_ensure_remote_state (self, op->kind, op->remote, NULL, error);
      if (state == NULL)
        return FALSE;
      if (transaction_is_local_only (self, op->kind))
        {
          g_autoptr(GVariant) commit_data = flatpak_dir_read_latest_commit (priv->dir, op->remote, op->ref,
                                                                            &checksum, NULL, error);
          if (commit_data == NULL)
            return FALSE;
          resolve_op_from_commit (self, op, checksum, NULL, commit_data);
        }
      else
        {
          g_autoptr(GError) local_error = NULL;
          g_autoptr(GFile) sideload_path = NULL;
          if (op->commit != NULL)
            {
              checksum = g_strdup (op->commit);
              sideload_path = flatpak_remote_state_lookup_sideload_checksum (state, op->commit);
            }
          else
            {
              g_autofree char *latest_checksum = NULL;
              g_autoptr(GFile) latest_sideload_path = NULL;
              g_autofree char *local_checksum = NULL;
              guint64 latest_timestamp;
              g_autoptr(GVariant) local_commit_data = flatpak_dir_read_latest_commit (priv->dir, op->remote, op->ref,
                                                                                      &local_checksum, NULL, NULL);
              if (flatpak_dir_find_latest_rev (priv->dir, state, flatpak_decomposed_get_ref (op->ref), op->commit,
                                               &latest_checksum, &latest_timestamp, &latest_sideload_path,
                                               cancellable, &local_error))
                {
                  if (latest_sideload_path != NULL && local_commit_data && latest_timestamp != 0 &&
                      ostree_commit_get_timestamp (local_commit_data) > latest_timestamp)
                    {
                      g_debug ("Installed commit %s newer than sideloaded %s, ignoring", local_checksum, latest_checksum);
                      checksum = g_steal_pointer (&local_checksum);
                    }
                  else
                    {
                      checksum = g_steal_pointer (&latest_checksum);
                      sideload_path = g_steal_pointer (&latest_sideload_path);
                    }
                }
              else
                {
                  if (local_commit_data == NULL)
                    {
                      g_propagate_error (error, g_steal_pointer (&local_error));
                      return FALSE;
                    }
                  g_message (_("Warning: Treating remote fetch error as non-fatal since %s is already installed: %s"),
                             flatpak_decomposed_get_ref (op->ref), local_error->message);
                  g_clear_error (&local_error);
                  checksum = g_steal_pointer (&local_checksum);
                }
            }
          if (!try_resolve_op_from_metadata (self, op, checksum, sideload_path, state))
            {
              g_autoptr(GVariant) commit_data = NULL;
              VarRefInfoRef ref_info;
              if (op->summary_metadata == NULL &&
                  flatpak_remote_state_lookup_ref (state, flatpak_decomposed_get_ref (op->ref),
                                                   NULL, NULL, &ref_info, NULL, NULL))
                op->summary_metadata = var_metadata_dup_to_gvariant (var_ref_info_get_metadata (ref_info));
              commit_data = flatpak_remote_state_load_ref_commit (state, priv->dir,
                                                                  flatpak_decomposed_get_ref (op->ref),
                                                                  checksum,   op->resolved_token,
                                                                  NULL, NULL, &local_error);
              if (commit_data == NULL)
                {
                  if (g_error_matches (local_error, FLATPAK_HTTP_ERROR, FLATPAK_HTTP_ERROR_UNAUTHORIZED) && !op->requested_token)
                    {
                      g_debug ("Unauthorized access during resolve by commit of %s, retrying with token", flatpak_decomposed_get_ref (op->ref));
                      priv->needs_resolve = TRUE;
                      priv->needs_tokens = TRUE;
                      op->token_type = G_MAXINT32;
                      op->resolved_commit = g_strdup (checksum);
                      g_clear_error (&local_error);
                      continue;
                    }
                  g_propagate_error (error, g_steal_pointer (&local_error));
                  return FALSE;
                }
              resolve_op_from_commit (self, op, checksum, sideload_path, commit_data);
            }
        }
    }
  return TRUE;
}