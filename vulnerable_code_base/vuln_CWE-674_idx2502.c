int unlinkat_harder(int dfd, const char *filename, int unlink_flags, RemoveFlags remove_flags) {
        mode_t old_mode;
        int r;
        if (unlinkat(dfd, filename, unlink_flags) >= 0)
                return 0;
        if (errno != EACCES || !FLAGS_SET(remove_flags, REMOVE_CHMOD))
                return -errno;
        r = patch_dirfd_mode(dfd, &old_mode);
        if (r < 0)
                return r;
        if (unlinkat(dfd, filename, unlink_flags) < 0) {
                r = -errno;
                (void) fchmod(dfd, old_mode);
                return r;
        }
        if (FLAGS_SET(remove_flags, REMOVE_CHMOD_RESTORE) && fchmod(dfd, old_mode) < 0)
                return -errno;
        return 0;
}