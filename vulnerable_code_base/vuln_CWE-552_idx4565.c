fu_plugin_set_secure_config_value(FuPlugin *self,
				  const gchar *key,
				  const gchar *value,
				  GError **error)
{
	g_autofree gchar *conf_path = fu_plugin_get_config_filename(self);
	gint ret;
	g_return_val_if_fail(FU_IS_PLUGIN(self), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!g_file_test(conf_path, G_FILE_TEST_EXISTS)) {
		g_set_error(error, FWUPD_ERROR, FWUPD_ERROR_NOT_FOUND, "%s is missing", conf_path);
		return FALSE;
	}
	ret = g_chmod(conf_path, 0660);
	if (ret == -1) {
		g_set_error(error,
			    FWUPD_ERROR,
			    FWUPD_ERROR_INTERNAL,
			    "failed to set permissions on %s",
			    conf_path);
		return FALSE;
	}
	return fu_plugin_set_config_value(self, key, value, error);
}