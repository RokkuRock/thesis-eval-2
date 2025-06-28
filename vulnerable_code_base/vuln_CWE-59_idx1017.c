static int fsmMkfile(rpmfi fi, const char *dest, rpmfiles files,
		     rpmpsm psm, int nodigest, int *setmeta,
		     int * firsthardlink)
{
    int rc = 0;
    int numHardlinks = rpmfiFNlink(fi);
    if (numHardlinks > 1) {
	if (*firsthardlink < 0) {
	    *firsthardlink = rpmfiFX(fi);
	    rc = expandRegular(fi, dest, psm, nodigest, 1);
	} else {
	    char *fn = rpmfilesFN(files, *firsthardlink);
	    rc = link(fn, dest);
	    if (rc < 0) {
		rc = RPMERR_LINK_FAILED;
	    }
	    free(fn);
	}
    }
    if (numHardlinks<=1) {
	if (!rc)
	    rc = expandRegular(fi, dest, psm, nodigest, 0);
    } else if (rpmfiArchiveHasContent(fi)) {
	if (!rc)
	    rc = expandRegular(fi, dest, psm, nodigest, 0);
	*firsthardlink = -1;
    } else {
	*setmeta = 0;
    }
    return rc;
}