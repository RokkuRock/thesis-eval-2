flatpak_dir_deploy (FlatpakDir          *self,
                    const char          *origin,
                    FlatpakDecomposed   *ref,
                    const char          *checksum_or_latest,
                    const char * const * subpaths,
                    const char * const * previous_ids,
                    GCancellable        *cancellable,
                    GError             **error)
{
  g_autofree char *resolved_ref = NULL;
  g_autofree char *ref_id = NULL;
  g_autoptr(GFile) root = NULL;
  g_autoptr(GFile) deploy_base = NULL;
  g_autoptr(GFile) checkoutdir = NULL;
  g_autoptr(GFile) bindir = NULL;
  g_autofree char *checkoutdirpath = NULL;
  g_autoptr(GFile) real_checkoutdir = NULL;
  g_autoptr(GFile) dotref = NULL;
  g_autoptr(GFile) files_etc = NULL;
  g_autoptr(GFile) deploy_data_file = NULL;
  g_autoptr(GVariant) commit_data = NULL;
  g_autoptr(GBytes) deploy_data = NULL;
  g_autoptr(GFile) export = NULL;
  g_autoptr(GFile) extradir = NULL;
  g_autoptr(GKeyFile) keyfile = NULL;
  guint64 installed_size = 0;
  OstreeRepoCheckoutAtOptions options = { 0, };
  const char *checksum;
  glnx_autofd int checkoutdir_dfd = -1;
  g_autoptr(GFile) tmp_dir_template = NULL;
  g_autofree char *tmp_dir_path = NULL;
  const char *xa_ref = NULL;
  g_autofree char *checkout_basename = NULL;
  gboolean created_extra_data = FALSE;
  g_autoptr(GVariant) commit_metadata = NULL;
  g_auto(GLnxLockFile) lock = { 0, };
  g_autoptr(GFile) metadata_file = NULL;
  g_autofree char *metadata_contents = NULL;
  gboolean is_oci;
  const char *flatpak;
  if (!flatpak_dir_ensure_repo (self, cancellable, error))
    return FALSE;
  ref_id = flatpak_decomposed_dup_id (ref);
  if (!flatpak_dir_repo_lock (self, &lock, LOCK_SH, cancellable, error))
    return FALSE;
  deploy_base = flatpak_dir_get_deploy_dir (self, ref);
  if (checksum_or_latest == NULL)
    {
      g_debug ("No checksum specified, getting tip of %s from origin %s", flatpak_decomposed_get_ref (ref), origin);
      resolved_ref = flatpak_dir_read_latest (self, origin, flatpak_decomposed_get_ref (ref), NULL, cancellable, error);
      if (resolved_ref == NULL)
        {
          g_prefix_error (error, _("While trying to resolve ref %s: "), flatpak_decomposed_get_ref (ref));
          return FALSE;
        }
      checksum = resolved_ref;
      g_debug ("tip resolved to: %s", checksum);
    }
  else
    {
      checksum = checksum_or_latest;
      g_debug ("Looking for checksum %s in local repo", checksum);
      if (!ostree_repo_read_commit (self->repo, checksum, NULL, NULL, cancellable, NULL))
        return flatpak_fail_error (error, FLATPAK_ERROR_INVALID_DATA, _("%s is not available"), flatpak_decomposed_get_ref (ref));
    }
  if (!ostree_repo_load_commit (self->repo, checksum, &commit_data, NULL, error))
    return FALSE;
  commit_metadata = g_variant_get_child_value (commit_data, 0);
  checkout_basename = flatpak_dir_get_deploy_subdir (self, checksum, subpaths);
  real_checkoutdir = g_file_get_child (deploy_base, checkout_basename);
  if (g_file_query_exists (real_checkoutdir, cancellable))
    return flatpak_fail_error (error, FLATPAK_ERROR_ALREADY_INSTALLED,
                               _("%s commit %s already installed"), flatpak_decomposed_get_ref (ref), checksum);
  g_autofree char *template = g_strdup_printf (".%s-XXXXXX", checkout_basename);
  tmp_dir_template = g_file_get_child (deploy_base, template);
  tmp_dir_path = g_file_get_path (tmp_dir_template);
  if (g_mkdtemp_full (tmp_dir_path, 0755) == NULL)
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED,
                           _("Can't create deploy directory"));
      return FALSE;
    }
  checkoutdir = g_file_new_for_path (tmp_dir_path);
  if (!ostree_repo_read_commit (self->repo, checksum, &root, NULL, cancellable, error))
    {
      g_prefix_error (error, _("Failed to read commit %s: "), checksum);
      return FALSE;
    }
  if (!flatpak_repo_collect_sizes (self->repo, root, &installed_size, NULL, cancellable, error))
    return FALSE;
  options.mode = OSTREE_REPO_CHECKOUT_MODE_USER;
  options.overwrite_mode = OSTREE_REPO_CHECKOUT_OVERWRITE_UNION_FILES;
  options.enable_fsync = FALSE;  
  options.bareuseronly_dirs = TRUE;  
  checkoutdirpath = g_file_get_path (checkoutdir);
  if (subpaths == NULL || *subpaths == NULL)
    {
      if (!ostree_repo_checkout_at (self->repo, &options,
                                    AT_FDCWD, checkoutdirpath,
                                    checksum,
                                    cancellable, error))
        {
          g_prefix_error (error, _("While trying to checkout %s into %s: "), checksum, checkoutdirpath);
          return FALSE;
        }
    }
  else
    {
      g_autoptr(GFile) files = g_file_get_child (checkoutdir, "files");
      int i;
      if (!g_file_make_directory_with_parents (files, cancellable, error))
        return FALSE;
      options.subpath = "/metadata";
      if (!ostree_repo_checkout_at (self->repo, &options,
                                    AT_FDCWD, checkoutdirpath,
                                    checksum,
                                    cancellable, error))
        {
          g_prefix_error (error, _("While trying to checkout metadata subpath: "));
          return FALSE;
        }
      for (i = 0; subpaths[i] != NULL; i++)
        {
          g_autofree char *subpath = g_build_filename ("/files", subpaths[i], NULL);
          g_autofree char *dstpath = g_build_filename (checkoutdirpath, "/files", subpaths[i], NULL);
          g_autofree char *dstpath_parent = g_path_get_dirname (dstpath);
          g_autoptr(GFile) child = NULL;
          child = g_file_resolve_relative_path (root, subpath);
          if (!g_file_query_exists (child, cancellable))
            {
              g_debug ("subpath %s not in tree", subpaths[i]);
              continue;
            }
          if (g_mkdir_with_parents (dstpath_parent, 0755))
            {
              glnx_set_error_from_errno (error);
              return FALSE;
            }
          options.subpath = subpath;
          if (!ostree_repo_checkout_at (self->repo, &options,
                                        AT_FDCWD, dstpath,
                                        checksum,
                                        cancellable, error))
            {
              g_prefix_error (error, _("While trying to checkout subpath ‘%s’: "), subpath);
              return FALSE;
            }
        }
    }
  extradir = g_file_resolve_relative_path (checkoutdir, "files/extra");
  if (!flatpak_rm_rf (extradir, cancellable, error))
    {
      g_prefix_error (error, _("While trying to remove existing extra dir: "));
      return FALSE;
    }
  if (!extract_extra_data (self, checksum, extradir, &created_extra_data, cancellable, error))
    return FALSE;
  if (created_extra_data)
    {
      if (!apply_extra_data (self, checkoutdir, cancellable, error))
        {
          g_prefix_error (error, _("While trying to apply extra data: "));
          return FALSE;
        }
    }
  g_variant_lookup (commit_metadata, "xa.ref", "&s", &xa_ref);
  if (xa_ref != NULL)
    {
      gboolean gpg_verify_summary;
      if (!ostree_repo_remote_get_gpg_verify_summary (self->repo, origin, &gpg_verify_summary, error))
        return FALSE;
      if (gpg_verify_summary)
        {
          FlatpakDecomposed *checkout_ref = ref;
          g_autoptr(FlatpakDecomposed) commit_ref = NULL;
          commit_ref = flatpak_decomposed_new_from_ref (xa_ref, error);
          if (commit_ref == NULL)
            {
              g_prefix_error (error, _("Invalid commit ref %s: "), xa_ref);
              return FALSE;
            }
          if (!flatpak_decomposed_equal_except_branch (checkout_ref, commit_ref))
            {
              g_set_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                           _("Deployed ref %s does not match commit (%s)"),
                           flatpak_decomposed_get_ref (ref), xa_ref);
              return FALSE;
            }
          if (strcmp (flatpak_decomposed_get_branch (checkout_ref), flatpak_decomposed_get_branch (commit_ref)) != 0)
            g_warning (_("Deployed ref %s branch does not match commit (%s)"),
                       flatpak_decomposed_get_ref (ref), xa_ref);
        }
      else if (strcmp (flatpak_decomposed_get_ref (ref), xa_ref) != 0)
        {
          g_set_error (error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED,
                       _("Deployed ref %s does not match commit (%s)"), flatpak_decomposed_get_ref (ref), xa_ref);
          return FALSE;
        }
    }
  keyfile = g_key_file_new ();
  metadata_file = g_file_resolve_relative_path (checkoutdir, "metadata");
  if (g_file_load_contents (metadata_file, NULL,
                            &metadata_contents, NULL, NULL, NULL))
    {
      if (!g_key_file_load_from_data (keyfile,
                                      metadata_contents,
                                      -1,
                                      0, error))
        return FALSE;
      if (!flatpak_check_required_version (flatpak_decomposed_get_ref (ref), keyfile, error))
        return FALSE;
    }
  is_oci = flatpak_dir_get_remote_oci (self, origin);
  if (!validate_commit_metadata (commit_data, flatpak_decomposed_get_ref (ref),
                                 metadata_contents, !is_oci, error))
    return FALSE;
  dotref = g_file_resolve_relative_path (checkoutdir, "files/.ref");
  if (!g_file_replace_contents (dotref, "", 0, NULL, FALSE,
                                G_FILE_CREATE_REPLACE_DESTINATION, NULL, cancellable, error))
    return FALSE;
  export = g_file_get_child (checkoutdir, "export");
  bindir = g_file_get_child (export, "bin");
  if (!flatpak_rm_rf (bindir, cancellable, error))
    return FALSE;
  if (flatpak_decomposed_is_runtime (ref))
    {
      files_etc = g_file_resolve_relative_path (checkoutdir, "files/etc");
      if (g_file_query_exists (files_etc, cancellable))
        {
          char *etcfiles[] = {"passwd", "group", "machine-id" };
          g_autoptr(GFile) etc_resolve_conf = g_file_get_child (files_etc, "resolv.conf");
          int i;
          for (i = 0; i < G_N_ELEMENTS (etcfiles); i++)
            {
              g_autoptr(GFile) etc_file = g_file_get_child (files_etc, etcfiles[i]);
              GFileType type;
              type = g_file_query_file_type (etc_file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                             cancellable);
              if (type == G_FILE_TYPE_REGULAR)
                continue;
              if (type != G_FILE_TYPE_UNKNOWN)
                {
                  if (!g_file_delete (etc_file, cancellable, error))
                    return FALSE;
                }
              if (!g_file_replace_contents (etc_file, "", 0, NULL, FALSE,
                                            G_FILE_CREATE_REPLACE_DESTINATION,
                                            NULL, cancellable, error))
                return FALSE;
            }
          if (g_file_query_exists (etc_resolve_conf, cancellable) &&
              !g_file_delete (etc_resolve_conf, cancellable, error))
            return FALSE;
          if (!g_file_make_symbolic_link (etc_resolve_conf,
                                          "/run/host/monitor/resolv.conf",
                                          cancellable, error))
            return FALSE;
        }
      if (!flatpak_rm_rf (export, cancellable, error))
        return FALSE;
    }
  else  
    {
      g_autofree char *ref_arch = flatpak_decomposed_dup_arch (ref);
      g_autofree char *ref_branch = flatpak_decomposed_dup_branch (ref);
      g_autoptr(GFile) wrapper = g_file_get_child (bindir, ref_id);
      g_autofree char *escaped_app = maybe_quote (ref_id);
      g_autofree char *escaped_branch = maybe_quote (ref_branch);
      g_autofree char *escaped_arch = maybe_quote (ref_arch);
      g_autofree char *bin_data = NULL;
      int r;
      if (!flatpak_mkdir_p (bindir, cancellable, error))
        return FALSE;
      if (!flatpak_rewrite_export_dir (ref_id, ref_branch, ref_arch,
                                       keyfile, previous_ids, export,
                                       cancellable,
                                       error))
        return FALSE;
      if ((flatpak = g_getenv ("FLATPAK_BINARY")) == NULL)
        flatpak = FLATPAK_BINDIR "/flatpak";
      bin_data = g_strdup_printf ("#!/bin/sh\nexec %s run --branch=%s --arch=%s %s \"$@\"\n",
                                  flatpak, escaped_branch, escaped_arch, escaped_app);
      if (!g_file_replace_contents (wrapper, bin_data, strlen (bin_data), NULL, FALSE,
                                    G_FILE_CREATE_REPLACE_DESTINATION, NULL, cancellable, error))
        return FALSE;
      do
        r = fchmodat (AT_FDCWD, flatpak_file_get_path_cached (wrapper), 0755, 0);
      while (G_UNLIKELY (r == -1 && errno == EINTR));
      if (r == -1)
        return glnx_throw_errno_prefix (error, "fchmodat");
    }
  deploy_data = flatpak_dir_new_deploy_data (self,
                                             checkoutdir,
                                             commit_data,
                                             commit_metadata,
                                             keyfile,
                                             ref_id,
                                             origin,
                                             checksum,
                                             (char **) subpaths,
                                             installed_size,
                                             previous_ids);
  if (!flatpak_dir_check_parental_controls (self, flatpak_decomposed_get_ref (ref), deploy_data, cancellable, error))
    return FALSE;
  deploy_data_file = g_file_get_child (checkoutdir, "deploy");
  if (!flatpak_bytes_save (deploy_data_file, deploy_data, cancellable, error))
    return FALSE;
  if (!glnx_opendirat (AT_FDCWD, checkoutdirpath, TRUE, &checkoutdir_dfd, error))
    return FALSE;
  if (syncfs (checkoutdir_dfd) != 0)
    {
      glnx_set_error_from_errno (error);
      return FALSE;
    }
  if (!g_file_move (checkoutdir, real_checkoutdir, G_FILE_COPY_NO_FALLBACK_FOR_MOVE,
                    cancellable, NULL, NULL, error))
    return FALSE;
  if (!flatpak_dir_set_active (self, ref, checkout_basename, cancellable, error))
    return FALSE;
  if (!flatpak_dir_update_deploy_ref (self, flatpak_decomposed_get_ref (ref), checksum, error))
    return FALSE;
  return TRUE;
}