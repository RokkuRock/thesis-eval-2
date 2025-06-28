thunar_transfer_job_copy_node (ThunarTransferJob  *job,
                               ThunarTransferNode *node,
                               GFile              *target_file,
                               GFile              *target_parent_file,
                               GList             **target_file_list_return,
                               GError            **error)
{
  ThunarThumbnailCache *thumbnail_cache;
  ThunarApplication    *application;
  ThunarJobResponse     response;
  GFileInfo            *info;
  GError               *err = NULL;
  GFile                *real_target_file = NULL;
  gchar                *base_name;
  _thunar_return_if_fail (THUNAR_IS_TRANSFER_JOB (job));
  _thunar_return_if_fail (node != NULL && G_IS_FILE (node->source_file));
  _thunar_return_if_fail (target_file == NULL || node->next == NULL);
  _thunar_return_if_fail ((target_file == NULL && target_parent_file != NULL) || (target_file != NULL && target_parent_file == NULL));
  _thunar_return_if_fail (error == NULL || *error == NULL);
  application = thunar_application_get ();
  thumbnail_cache = thunar_application_get_thumbnail_cache (application);
  g_object_unref (application);
  for (; err == NULL && node != NULL; node = node->next)
    {
      if (G_LIKELY (target_file == NULL))
        {
          base_name = g_file_get_basename (node->source_file);
          target_file = g_file_get_child (target_parent_file, base_name);
          g_free (base_name);
        }
      else
        target_file = g_object_ref (target_file);
      info = g_file_query_info (node->source_file,
                                G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                exo_job_get_cancellable (EXO_JOB (job)),
                                &err);
      if (info == NULL)
        {
          g_object_unref (target_file);
          break;
        }
      exo_job_info_message (EXO_JOB (job), g_file_info_get_display_name (info));
retry_copy:
      real_target_file = thunar_transfer_job_copy_file (job, node->source_file, 
                                                        target_file, &err);
      if (G_LIKELY (real_target_file != NULL))
        {
          if (G_LIKELY (node->source_file != real_target_file))
            {
              thunar_thumbnail_cache_copy_file (thumbnail_cache, 
                                                node->source_file, 
                                                real_target_file);
              if (node->children != NULL)
                {
                  thunar_transfer_job_copy_node (job, node->children, NULL, real_target_file, NULL, &err);
                  thunar_transfer_node_free (node->children);
                  node->children = NULL;
                }
              if (G_UNLIKELY (err != NULL))
                {
                  g_object_unref (real_target_file);
                  g_object_unref (target_file);
                  break;
                }
              if (G_LIKELY (target_file_list_return != NULL))
                {
                  *target_file_list_return = 
                    thunar_g_file_list_prepend (*target_file_list_return, 
                                                real_target_file);
                }
retry_remove:
              if (job->type == THUNAR_TRANSFER_JOB_MOVE)
                {
                  if (g_file_delete (node->source_file, 
                                     exo_job_get_cancellable (EXO_JOB (job)), 
                                     &err))
                    {
                      thunar_thumbnail_cache_delete_file (thumbnail_cache, 
                                                          node->source_file);
                    }
                  else
                    {
                      response = thunar_job_ask_skip (THUNAR_JOB (job), "%s", 
                                                      err->message);
                      g_clear_error (&err);
                      if (G_UNLIKELY (response == THUNAR_JOB_RESPONSE_RETRY))
                        goto retry_remove;
                    }
                }
            }
          g_object_unref (real_target_file);
        }
      else if (err != NULL)
        { 
          if (err->domain != G_IO_ERROR || err->code != G_IO_ERROR_NO_SPACE) 
            {
              response = thunar_job_ask_skip (THUNAR_JOB (job), "%s", err->message);
              g_clear_error (&err);
              if (G_UNLIKELY (response == THUNAR_JOB_RESPONSE_RETRY))
                goto retry_copy;
            }
        }
      g_object_unref (target_file);
      target_file = NULL;
      g_object_unref (info);
    }
  g_object_unref (thumbnail_cache);
  if (G_UNLIKELY (err != NULL))
    g_propagate_error (error, err);
}