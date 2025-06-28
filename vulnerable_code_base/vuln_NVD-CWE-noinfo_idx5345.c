static inline FILE *xfmkstemp(char **tmpname, char *dir)
{
	int fd;
	FILE *ret;
	fd = xmkstemp(tmpname, dir);
	if (fd == -1)
		return NULL;
	if (!(ret = fdopen(fd, "w+" UL_CLOEXECSTR))) {
		close(fd);
		return NULL;
	}
	return ret;
}