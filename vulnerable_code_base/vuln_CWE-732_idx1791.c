M_fs_error_t M_fs_delete(const char *path, M_bool remove_children, M_fs_progress_cb_t cb, M_uint32 progress_flags)
{
	char                   *norm_path;
	char                   *join_path;
	M_fs_dir_entries_t     *entries;
	const M_fs_dir_entry_t *entry;
	M_fs_info_t            *info;
	M_fs_progress_t        *progress            = NULL;
	M_fs_dir_walk_filter_t  filter              = M_FS_DIR_WALK_FILTER_ALL|M_FS_DIR_WALK_FILTER_RECURSE;
	M_fs_type_t             type;
	M_fs_error_t            res;
	M_fs_error_t            res2;
	size_t                  len;
	size_t                  i;
	M_uint64                total_size          = 0;
	M_uint64                total_size_progress = 0;
	M_uint64                entry_size;
	res = M_fs_path_norm(&norm_path, path, M_FS_PATH_NORM_HOME, M_FS_SYSTEM_AUTO);
	if (res != M_FS_ERROR_SUCCESS) {
		M_free(norm_path);
		return res;
	}
	res = M_fs_info(&info, norm_path, M_FS_PATH_INFO_FLAGS_BASIC);
	if (res != M_FS_ERROR_SUCCESS) {
		M_free(norm_path);
		return res;
	}
	type = M_fs_info_get_type(info);
	if (type == M_FS_TYPE_UNKNOWN) {
		M_fs_info_destroy(info);
		M_free(norm_path);
		return M_FS_ERROR_GENERIC;
	}
	entries = M_fs_dir_entries_create();
	if (type == M_FS_TYPE_DIR && remove_children) {
		if (cb && progress_flags & (M_FS_PROGRESS_SIZE_TOTAL|M_FS_PROGRESS_SIZE_CUR)) {
			filter |= M_FS_DIR_WALK_FILTER_READ_INFO_BASIC;
		}
		M_fs_dir_entries_merge(&entries, M_fs_dir_walk_entries(norm_path, NULL, filter));
	}
	M_fs_dir_entries_insert(entries, M_fs_dir_walk_fill_entry(norm_path, NULL, type, info, M_FS_DIR_WALK_FILTER_READ_INFO_BASIC));
	len = M_fs_dir_entries_len(entries);
	if (cb) {
		progress = M_fs_progress_create();
		if (progress_flags & M_FS_PROGRESS_SIZE_TOTAL) {
			for (i=0; i<len; i++) {
				entry       = M_fs_dir_entries_at(entries, i);
				entry_size  = M_fs_info_get_size(M_fs_dir_entry_get_info(entry));
				total_size += entry_size;
			}
			M_fs_progress_set_size_total(progress, total_size);
		}
		if (progress_flags & M_FS_PROGRESS_COUNT) {
			M_fs_progress_set_count_total(progress, len);
		}
	}
	res = M_FS_ERROR_SUCCESS;
	for (i=0; i<len; i++) {
		entry     = M_fs_dir_entries_at(entries, i);
		join_path = M_fs_path_join(norm_path, M_fs_dir_entry_get_name(entry), M_FS_SYSTEM_AUTO);
		if (M_fs_dir_entry_get_type(entry) == M_FS_TYPE_DIR) {
			res2 = M_fs_delete_dir(join_path);
		} else {
			res2 = M_fs_delete_file(join_path);
		}
		if (res2 != M_FS_ERROR_SUCCESS) {
			res = M_FS_ERROR_GENERIC;
		}
		if (cb) {
			entry_size           = M_fs_info_get_size(M_fs_dir_entry_get_info(entry));
			total_size_progress += entry_size;
			M_fs_progress_set_path(progress, join_path);
			M_fs_progress_set_type(progress, M_fs_dir_entry_get_type(entry));
			M_fs_progress_set_result(progress, res2);
			if (progress_flags & M_FS_PROGRESS_COUNT) {
				M_fs_progress_set_count(progress, i+1);
			}
			if (progress_flags & M_FS_PROGRESS_SIZE_TOTAL) {
				M_fs_progress_set_size_total_progess(progress, total_size_progress);
			}
			if (progress_flags & M_FS_PROGRESS_SIZE_CUR) {
				M_fs_progress_set_size_current(progress, entry_size);
				M_fs_progress_set_size_current_progress(progress, entry_size);
			}
		}
		M_free(join_path);
		if (cb && !cb(progress)) {
			res = M_FS_ERROR_CANCELED;
			break;
		}
	}
	M_fs_dir_entries_destroy(entries);
	M_fs_progress_destroy(progress);
	M_free(norm_path);
	return res;
}