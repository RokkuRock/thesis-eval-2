gvdb_table_write_contents_async (GHashTable          *table,
                                 const gchar         *filename,
                                 gboolean             byteswap,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
  struct gvdb_pointer root;
  FileBuilder *fb;
  WriteContentsData *data;
  GString *str;
  GBytes *bytes;
  GFile *file;
  GTask *task;
  g_return_if_fail (table != NULL);
  g_return_if_fail (filename != NULL);
  g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
  fb = file_builder_new (byteswap);
  file_builder_add_hash (fb, table, &root);
  str = file_builder_serialise (fb, root);
  bytes = g_string_free_to_bytes (str);
  file_builder_free (fb);
  file = g_file_new_for_path (filename);
  data = write_contents_data_new (bytes, file);
  task = g_task_new (NULL, cancellable, callback, user_data);
  g_task_set_task_data (task, data, (GDestroyNotify)write_contents_data_free);
  g_task_set_source_tag (task, gvdb_table_write_contents_async);
  g_file_replace_contents_async (file, str->str, str->len,
                                 NULL, FALSE,
                                 G_FILE_CREATE_PRIVATE | G_FILE_CREATE_REPLACE_DESTINATION,
                                 cancellable, replace_contents_cb, g_steal_pointer (&task));
  g_bytes_unref (bytes);
  g_object_unref (file);
}