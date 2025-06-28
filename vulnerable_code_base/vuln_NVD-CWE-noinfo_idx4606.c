static FILE * pw_tmpfile(int lockfd)
{
	FILE *fd;
	char *tmpname = NULL;
	char *dir = "/etc";
	if ((fd = xfmkstemp(&tmpname, dir)) == NULL) {
		ulckpwdf();
		err(EXIT_FAILURE, _("can't open temporary file"));
	}
	copyfile(lockfd, fileno(fd));
	tmp_file = tmpname;
	return fd;
}