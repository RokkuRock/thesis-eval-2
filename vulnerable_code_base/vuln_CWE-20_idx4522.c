check_symlinks(struct archive_write_disk *a)
{
#if !defined(HAVE_LSTAT)
	(void)a;  
	return (ARCHIVE_OK);
#else
	char *pn;
	char c;
	int r;
	struct stat st;
	pn = a->name;
	if (archive_strlen(&(a->path_safe)) > 0) {
		char *p = a->path_safe.s;
		while ((*pn != '\0') && (*p == *pn))
			++p, ++pn;
	}
	if(pn == a->name && pn[0] == '/')
		++pn;
	c = pn[0];
	while (pn[0] != '\0' && (pn[0] != '/' || pn[1] != '\0')) {
		while (*pn != '\0' && *pn != '/')
			++pn;
		c = pn[0];
		pn[0] = '\0';
		r = lstat(a->name, &st);
		if (r != 0) {
			if (errno == ENOENT) {
				break;
			} else {
				return (ARCHIVE_FAILED);
			}
		} else if (S_ISLNK(st.st_mode)) {
			if (c == '\0') {
				if (unlink(a->name)) {
					archive_set_error(&a->archive, errno,
					    "Could not remove symlink %s",
					    a->name);
					pn[0] = c;
					return (ARCHIVE_FAILED);
				}
				a->pst = NULL;
				if (!S_ISLNK(a->mode)) {
					archive_set_error(&a->archive, 0,
					    "Removing symlink %s",
					    a->name);
				}
				pn[0] = c;
				return (0);
			} else if (a->flags & ARCHIVE_EXTRACT_UNLINK) {
				if (unlink(a->name) != 0) {
					archive_set_error(&a->archive, 0,
					    "Cannot remove intervening symlink %s",
					    a->name);
					pn[0] = c;
					return (ARCHIVE_FAILED);
				}
				a->pst = NULL;
			} else {
				archive_set_error(&a->archive, 0,
				    "Cannot extract through symlink %s",
				    a->name);
				pn[0] = c;
				return (ARCHIVE_FAILED);
			}
		}
		pn[0] = c;
		if (pn[0] != '\0')
			pn++;  
	}
	pn[0] = c;
	archive_strcpy(&a->path_safe, a->name);
	return (ARCHIVE_OK);
#endif
}