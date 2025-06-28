int rm_rf_children(
                int fd,
                RemoveFlags flags,
                const struct stat *root_dev) {
        _cleanup_closedir_ DIR *d = NULL;
        int ret = 0, r;
        assert(fd >= 0);
        d = fdopendir(fd);
        if (!d) {
                safe_close(fd);
                return -errno;
        }
        if (!(flags & REMOVE_PHYSICAL)) {
                struct statfs sfs;
                if (fstatfs(dirfd(d), &sfs) < 0)
                        return -errno;
                if (is_physical_fs(&sfs)) {
                        _cleanup_free_ char *path = NULL;
                        (void) fd_get_path(fd, &path);
                        return log_error_errno(SYNTHETIC_ERRNO(EPERM),
                                               "Attempted to remove disk file system under \"%s\", and we can't allow that.",
                                               strna(path));
                }
        }
        FOREACH_DIRENT_ALL(de, d, return -errno) {
                int is_dir;
                if (dot_or_dot_dot(de->d_name))
                        continue;
                is_dir =
                        de->d_type == DT_UNKNOWN ? -1 :
                        de->d_type == DT_DIR;
                r = rm_rf_children_inner(dirfd(d), de->d_name, is_dir, flags, root_dev);
                if (r < 0 && r != -ENOENT && ret == 0)
                        ret = r;
        }
        if (FLAGS_SET(flags, REMOVE_SYNCFS) && syncfs(dirfd(d)) < 0 && ret >= 0)
                ret = -errno;
        return ret;
}