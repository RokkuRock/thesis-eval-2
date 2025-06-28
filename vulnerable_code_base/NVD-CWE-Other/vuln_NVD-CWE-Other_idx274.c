static int get_dirfd(char *path, bool symlinks)
{
	char *ch;
	char *item;
	char *linkpath = NULL;
	char *path_dupe;
	char *str;
	int components = 0;
	int dirfd;
	int flags = 0;
	int new_dirfd;
	struct stat st;
	ssize_t linksize;
	if (!path || *path != '/')
		eerrorx("%s: empty or relative path", applet);
	dirfd = openat(dirfd, "/", O_RDONLY);
	if (dirfd == -1)
		eerrorx("%s: unable to open the root directory: %s",
				applet, strerror(errno));
	path_dupe = xstrdup(path);
	ch = path_dupe;
	while (*ch) {
		if (*ch == '/')
			components++;
		ch++;
	}
	item = strtok(path_dupe, "/");
#ifdef O_PATH
	flags |= O_PATH;
#endif
	if (!symlinks)
		flags |= O_NOFOLLOW;
	flags |= O_RDONLY;
	while (dirfd > 0 && item && components > 1) {
		str = xstrdup(linkpath ? linkpath : item);
		new_dirfd = openat(dirfd, str, flags);
		if (new_dirfd == -1)
			eerrorx("%s: %s: could not open %s: %s", applet, path, str,
					strerror(errno));
		if (fstat(new_dirfd, &st) == -1)
			eerrorx("%s: %s: unable to stat %s: %s", applet, path, item,
					strerror(errno));
		if (S_ISLNK(st.st_mode) ) {
			if (st.st_uid != 0)
				eerrorx("%s: %s: symbolic link %s not owned by root",
						applet, path, str);
			linksize = st.st_size+1;
			if (linkpath)
				free(linkpath);
			linkpath = xmalloc(linksize);
			memset(linkpath, 0, linksize);
			if (readlinkat(new_dirfd, "", linkpath, linksize) != st.st_size)
				eerrorx("%s: symbolic link destination changed", applet);
			close(new_dirfd);
		} else {
			close(dirfd);
			dirfd = new_dirfd;
			free(linkpath);
			linkpath = NULL;
			item = strtok(NULL, "/");
			components--;
		}
	}
	free(path_dupe);
	if (linkpath) {
		free(linkpath);
		linkpath = NULL;
	}
	return dirfd;
}