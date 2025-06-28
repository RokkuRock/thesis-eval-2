find_sessions(const char *dir, regex_t *re, const char *user, const char *tty)
{
    DIR *d;
    struct dirent *dp;
    struct stat sb;
    size_t sdlen, sessions_len = 0, sessions_size = 0;
    unsigned int i;
    int len;
    char pathbuf[PATH_MAX], **sessions = NULL;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
    bool checked_type = true;
#else
    const bool checked_type = false;
#endif
    debug_decl(find_sessions, SUDO_DEBUG_UTIL);
    d = opendir(dir);
    if (d == NULL)
	sudo_fatal(U_("unable to open %s"), dir);
    sdlen = strlcpy(pathbuf, dir, sizeof(pathbuf));
    if (sdlen + 1 >= sizeof(pathbuf)) {
	errno = ENAMETOOLONG;
	sudo_fatal("%s/", dir);
    }
    pathbuf[sdlen++] = '/';
    pathbuf[sdlen] = '\0';
    while ((dp = readdir(d)) != NULL) {
	if (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
	    (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
	    continue;
#ifdef HAVE_STRUCT_DIRENT_D_TYPE
	if (checked_type) {
	    if (dp->d_type != DT_DIR) {
		if (dp->d_type != DT_UNKNOWN)
		    continue;
		checked_type = false;
	    }
	}
#endif
	if (sessions_len + 1 > sessions_size) {
	    if (sessions_size == 0)
		sessions_size = 36 * 36 / 2;
	    sessions = reallocarray(sessions, sessions_size, 2 * sizeof(char *));
	    if (sessions == NULL)
		sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    sessions_size *= 2;
	}
	if ((sessions[sessions_len] = strdup(dp->d_name)) == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sessions_len++;
    }
    closedir(d);
    if (sessions != NULL) {
	qsort(sessions, sessions_len, sizeof(char *), session_compare);
	for (i = 0; i < sessions_len; i++) {
	    len = snprintf(&pathbuf[sdlen], sizeof(pathbuf) - sdlen,
		"%s/log", sessions[i]);
	    if (len < 0 || (size_t)len >= sizeof(pathbuf) - sdlen) {
		errno = ENAMETOOLONG;
		sudo_fatal("%s/%s/log", dir, sessions[i]);
	    }
	    free(sessions[i]);
	    if (lstat(pathbuf, &sb) == 0 && S_ISREG(sb.st_mode)) {
		pathbuf[sdlen + len - 4] = '\0';
		list_session(pathbuf, re, user, tty);
	    } else {
		pathbuf[sdlen + len - 4] = '\0';
		if (checked_type ||
		    (lstat(pathbuf, &sb) == 0 && S_ISDIR(sb.st_mode)))
		    find_sessions(pathbuf, re, user, tty);
	    }
	}
	free(sessions);
    }
    debug_return_int(0);
}