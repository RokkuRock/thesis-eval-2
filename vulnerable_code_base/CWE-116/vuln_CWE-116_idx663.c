print_eol_info_message (FlatpakDir        *dir,
                        FlatpakDecomposed *ref,
                        const char        *ref_name,
                        const char        *rebased_to_ref,
                        const char        *reason)
{
  gboolean is_pinned = flatpak_dir_ref_is_pinned (dir, flatpak_decomposed_get_ref (ref));
  g_autofree char *ref_branch = flatpak_decomposed_dup_branch (ref);
  const char *on = "";
  const char *off = "";
  if (flatpak_fancy_output ())
    {
      on = FLATPAK_ANSI_BOLD_ON;
      off = FLATPAK_ANSI_BOLD_OFF;
    }
  if (rebased_to_ref)
    {
      g_autoptr(FlatpakDecomposed) eolr_decomposed = NULL;
      g_autofree char *eolr_name = NULL;
      const char *eolr_branch;
      eolr_decomposed = flatpak_decomposed_new_from_ref (rebased_to_ref, NULL);
      g_assert (eolr_decomposed != NULL);
      g_assert (flatpak_decomposed_get_kind (ref) == flatpak_decomposed_get_kind (eolr_decomposed));
      eolr_name = flatpak_decomposed_dup_id (eolr_decomposed);
      eolr_branch = flatpak_decomposed_get_branch (eolr_decomposed);
      if (is_pinned)
        {
          g_print (_("\nInfo: (pinned) runtime %s%s%s branch %s%s%s is end-of-life, in favor of %s%s%s branch %s%s%s\n"),
                   on, ref_name, off, on, ref_branch, off, on, eolr_name, off, on, eolr_branch, off);
        }
      else
        {
          if (flatpak_decomposed_is_runtime (ref))
            g_print (_("\nInfo: runtime %s%s%s branch %s%s%s is end-of-life, in favor of %s%s%s branch %s%s%s\n"),
                     on, ref_name, off, on, ref_branch, off, on, eolr_name, off, on, eolr_branch, off);
          else
            g_print (_("\nInfo: app %s%s%s branch %s%s%s is end-of-life, in favor of %s%s%s branch %s%s%s\n"),
                     on, ref_name, off, on, ref_branch, off, on, eolr_name, off, on, eolr_branch, off);
        }
    }
  else if (reason)
    {
      if (is_pinned)
        {
          g_print (_("\nInfo: (pinned) runtime %s%s%s branch %s%s%s is end-of-life, with reason:\n"),
                   on, ref_name, off, on, ref_branch, off);
        }
      else
        {
          if (flatpak_decomposed_is_runtime (ref))
            g_print (_("\nInfo: runtime %s%s%s branch %s%s%s is end-of-life, with reason:\n"),
                     on, ref_name, off, on, ref_branch, off);
          else
            g_print (_("\nInfo: app %s%s%s branch %s%s%s is end-of-life, with reason:\n"),
                     on, ref_name, off, on, ref_branch, off);
        }
      g_print ("   %s\n", reason);
    }
}