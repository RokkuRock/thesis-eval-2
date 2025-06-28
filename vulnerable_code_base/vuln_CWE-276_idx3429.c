flatpak_dir_pull (FlatpakDir                           *self,
                  FlatpakRemoteState                   *state,
                  const char                           *ref,
                  const char                           *opt_rev,
                  const char                          **subpaths,
                  GFile                                *sideload_repo,
                  GBytes                               *require_metadata,
                  const char                           *token,
                  OstreeRepo                           *repo,
                  FlatpakPullFlags                      flatpak_flags,
                  OstreeRepoPullFlags                   flags,
                  FlatpakProgress                      *progress,
                  GCancellable                         *cancellable,
                  GError                              **error)
{
  gboolean ret = FALSE;
  g_autofree char *rev = NULL;
  g_autofree char *url = NULL;
  g_autoptr(GPtrArray) subdirs_arg = NULL;
  g_auto(GLnxLockFile) lock = { 0, };
  g_autofree char *name = NULL;
  g_autofree char *current_checksum = NULL;
  if (!flatpak_dir_ensure_repo (self, cancellable, error))
    return FALSE;
  if (repo == NULL && !flatpak_dir_repo_lock (self, &lock, LOCK_SH, cancellable, error))
    return FALSE;
  if (flatpak_dir_get_remote_oci (self, state->remote_name))
    return flatpak_dir_pull_oci (self, state, ref, opt_rev, repo, flatpak_flags,
                                 flags, token, progress, cancellable, error);
  if (!ostree_repo_remote_get_url (self->repo,
                                   state->remote_name,
                                   &url,
                                   error))
    return FALSE;
  if (*url == 0)
    return TRUE;  
  if (opt_rev != NULL)
    {
      rev = g_strdup (opt_rev);
    }
  else
    {
      flatpak_remote_state_lookup_ref (state, ref, &rev, NULL, NULL, NULL, error);
      if (rev == NULL && error != NULL && *error == NULL)
        flatpak_fail_error (error, FLATPAK_ERROR_REF_NOT_FOUND, _("Couldn't find latest checksum for ref %s in remote %s"),
                            ref, state->remote_name);
      if (rev == NULL)
        {
          g_assert (error == NULL || *error != NULL);
          return FALSE;
        }
    }
  g_debug ("%s: Using commit %s for pull of ref %s from remote %s%s%s",
           G_STRFUNC, rev, ref, state->remote_name,
           sideload_repo ? "sideloaded from " : "",
           sideload_repo ? flatpak_file_get_path_cached (sideload_repo) : ""
           );
  if (repo == NULL)
    repo = self->repo;
  if (subpaths != NULL && subpaths[0] != NULL)
    {
      subdirs_arg = g_ptr_array_new_with_free_func (g_free);
      int i;
      g_ptr_array_add (subdirs_arg, g_strdup ("/metadata"));
      for (i = 0; subpaths[i] != NULL; i++)
        g_ptr_array_add (subdirs_arg,
                         g_build_filename ("/files", subpaths[i], NULL));
      g_ptr_array_add (subdirs_arg, NULL);
    }
  if (!flatpak_dir_setup_extra_data (self, state, repo,
                                     ref, rev, sideload_repo, token,
                                     flatpak_flags,
                                     progress,
                                     cancellable,
                                     error))
    goto out;
  if (!ostree_repo_prepare_transaction (repo, NULL, cancellable, error))
    goto out;
  flatpak_repo_resolve_rev (repo, NULL, state->remote_name, ref, TRUE,
                            &current_checksum, NULL, NULL);
  if (!repo_pull (repo, state,
                  subdirs_arg ? (const char **) subdirs_arg->pdata : NULL,
                  ref, rev, sideload_repo, token, flatpak_flags, flags,
                  progress,
                  cancellable, error))
    {
      g_prefix_error (error, _("While pulling %s from remote %s: "), ref, state->remote_name);
      goto out;
    }
  if (require_metadata)
    {
      g_autoptr(GVariant) commit_data = NULL;
      if (!ostree_repo_load_commit (repo, rev, &commit_data, NULL, error) ||
          !validate_commit_metadata (commit_data,
                                     ref,
                                     (const char *)g_bytes_get_data (require_metadata, NULL),
                                     g_bytes_get_size (require_metadata),
                                     TRUE,
                                     error))
        goto out;
    }
  if (!flatpak_dir_pull_extra_data (self, repo,
                                    state->remote_name,
                                    ref, rev,
                                    flatpak_flags,
                                    progress,
                                    cancellable,
                                    error))
    goto out;
  if (!ostree_repo_commit_transaction (repo, NULL, cancellable, error))
    goto out;
  ret = TRUE;
  if (repo == self->repo)
    name = flatpak_dir_get_name (self);
  else
    {
      GFile *file = ostree_repo_get_path (repo);
      name = g_file_get_path (file);
    }
  (flatpak_dir_log) (self, __FILE__, __LINE__, __FUNCTION__, name,
                     "pull", state->remote_name, ref, rev, current_checksum, NULL,
                     "Pulled %s from %s", ref, state->remote_name);
out:
  if (!ret)
    {
      ostree_repo_abort_transaction (repo, cancellable, NULL);
      g_assert (error == NULL || *error != NULL);
    }
  return ret;
}