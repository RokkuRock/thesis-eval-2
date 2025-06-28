guestfs___first_line_of_file (guestfs_h *g, const char *filename)
{
  CLEANUP_FREE char **lines = NULL;  
  int64_t size;
  char *ret;
  size = guestfs_filesize (g, filename);
  if (size == -1)
    return NULL;
  if (size > MAX_SMALL_FILE_SIZE) {
    error (g, _("size of %s is unreasonably large (%" PRIi64 " bytes)"),
           filename, size);
    return NULL;
  }
  lines = guestfs_head_n (g, 1, filename);
  if (lines == NULL)
    return NULL;
  if (lines[0] == NULL) {
    guestfs___free_string_list (lines);
    return safe_strdup (g, "");
  }
  ret = lines[0];                
  return ret;
}