int setpwnam(struct passwd *pwd)
{
	FILE *fp = NULL, *pwf = NULL;
	int save_errno;
	int found;
	int namelen;
	int buflen = 256;
	int contlen, rc;
	char *linebuf = NULL;
	char *tmpname = NULL;
	char *atomic_dir = "/etc";
	pw_init();
	if ((fp = xfmkstemp(&tmpname, atomic_dir)) == NULL)
		return -1;
	if (fchown(fileno(fp), (uid_t) 0, (gid_t) 0) < 0)
		goto fail;
	if (lckpwdf() < 0)
		goto fail;
	pwf = fopen(PASSWD_FILE, "r");
	if (!pwf)
		goto fail;
	namelen = strlen(pwd->pw_name);
	linebuf = malloc(buflen);
	if (!linebuf)
		goto fail;
	found = false;
	while (fgets(linebuf, buflen, pwf) != NULL) {
		contlen = strlen(linebuf);
		while (linebuf[contlen - 1] != '\n' && !feof(pwf)) {
			char *tmp;
			buflen *= 2;
			tmp = realloc(linebuf, buflen);
			if (tmp == NULL)
				goto fail;
			linebuf = tmp;
			if (fgets(&linebuf[contlen], buflen / 2, pwf) == NULL)
				break;
			contlen = strlen(linebuf);
		}
		if (!found && linebuf[namelen] == ':' &&
		    !strncmp(linebuf, pwd->pw_name, namelen)) {
			if (putpwent(pwd, fp) < 0)
				goto fail;
			found = true;
			continue;
		}
		fputs(linebuf, fp);
	}
	if (fchmod(fileno(fp), 0644) < 0)
		goto fail;
	rc = close_stream(fp);
	fp = NULL;
	if (rc != 0)
		goto fail;
	fclose(pwf);	 
	pwf = NULL;
	if (!found) {
		errno = ENOENT;	 
		goto fail;
	}
	unlink(PASSWD_FILE ".OLD");
	ignore_result(link(PASSWD_FILE, PASSWD_FILE ".OLD"));
	if (rename(tmpname, PASSWD_FILE) < 0)
		goto fail;
	ulckpwdf();
	return 0;
 fail:
	save_errno = errno;
	ulckpwdf();
	if (fp != NULL)
		fclose(fp);
	if (tmpname != NULL)
		unlink(tmpname);
	free(tmpname);
	if (pwf != NULL)
		fclose(pwf);
	free(linebuf);
	errno = save_errno;
	return -1;
}