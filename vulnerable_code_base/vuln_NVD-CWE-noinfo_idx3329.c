gimp_write_and_read_file (Gimp     *gimp,
                          gboolean  with_unusual_stuff,
                          gboolean  compat_paths,
                          gboolean  use_gimp_2_8_features)
{
  GimpImage           *image;
  GimpImage           *loaded_image;
  GimpPlugInProcedure *proc;
  gchar               *filename;
  GFile               *file;
  image = gimp_create_mainimage (gimp,
                                 with_unusual_stuff,
                                 compat_paths,
                                 use_gimp_2_8_features);
  gimp_assert_mainimage (image,
                         with_unusual_stuff,
                         compat_paths,
                         use_gimp_2_8_features);
  filename = g_build_filename (g_get_tmp_dir (), "gimp-test.xcf", NULL);
  file = g_file_new_for_path (filename);
  g_free (filename);
  proc = gimp_plug_in_manager_file_procedure_find (image->gimp->plug_in_manager,
                                                   GIMP_FILE_PROCEDURE_GROUP_SAVE,
                                                   file,
                                                   NULL  );
  file_save (gimp,
             image,
             NULL  ,
             file,
             proc,
             GIMP_RUN_NONINTERACTIVE,
             FALSE  ,
             FALSE  ,
             FALSE  ,
             NULL  );
  loaded_image = gimp_test_load_image (image->gimp, file);
  gimp_assert_mainimage (loaded_image,
                         with_unusual_stuff,
                         compat_paths,
                         use_gimp_2_8_features);
  g_file_delete (file, NULL, NULL);
  g_object_unref (file);
}