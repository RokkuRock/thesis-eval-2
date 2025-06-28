static int expandRegular(rpmfi fi, const char *dest, rpmpsm psm, int nodigest, int nocontent)
{
    FD_t wfd = NULL;
    int rc = 0;
    {
	mode_t old_umask = umask(0577);
	wfd = Fopen(dest, "w.ufdio");
	umask(old_umask);
    }
    if (Ferror(wfd)) {
	rc = RPMERR_OPEN_FAILED;
	goto exit;
    }
    if (!nocontent)
	rc = rpmfiArchiveReadToFilePsm(fi, wfd, nodigest, psm);
exit:
    if (wfd) {
	int myerrno = errno;
	Fclose(wfd);
	errno = myerrno;
    }
    return rc;
}