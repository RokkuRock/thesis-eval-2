char *M_fs_path_join_parts(const M_list_str_t *path, M_fs_system_t sys_type)
{
	M_list_str_t *parts;
	const char   *part;
	char         *out;
	size_t        len;
	size_t        i;
	size_t        count;
	if (path == NULL) {
		return NULL;
	}
	len = M_list_str_len(path);
	if (len == 0) {
		return NULL;
	}
	sys_type = M_fs_path_get_system_type(sys_type);
	parts = M_list_str_duplicate(path);
	for (i=len-1; i>0; i--) {
		part = M_list_str_at(parts, i);
		if (part == NULL || *part == '\0') {
			M_list_str_remove_at(parts, i);
		}
	}
	len = M_list_str_len(parts);
	part = M_list_str_at(parts, 0);
	if (len == 1 && (part == NULL || *part == '\0')) {
		M_list_str_destroy(parts);
		if (sys_type == M_FS_SYSTEM_WINDOWS) {
			return M_strdup("\\\\");
		}
		return M_strdup("/");
	}
	if (sys_type == M_FS_SYSTEM_WINDOWS && len > 0) {
		part  = M_list_str_at(parts, 0);
		count = (len == 1) ? 2 : 1;
		if (part != NULL && *part == '\0') {
			for (i=0; i<count; i++) {
				M_list_str_insert_at(parts, "", 0);
			}
		} else if (M_fs_path_isabs(part, sys_type) && len == 1) {
			M_list_str_insert_at(parts, "", 1);
		}
	}
	out = M_list_str_join(parts, (unsigned char)M_fs_path_get_system_sep(sys_type));
	M_list_str_destroy(parts);
	return out;
}