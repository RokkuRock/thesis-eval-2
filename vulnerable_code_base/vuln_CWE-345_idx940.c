rpmRC rpmReadPackageFile(rpmts ts, FD_t fd, const char * fn, Header * hdrp)
{
    char *msg = NULL;
    Header h = NULL;
    Header sigh = NULL;
    hdrblob blob = NULL;
    hdrblob sigblob = NULL;
    rpmVSFlags vsflags = rpmtsVSFlags(ts) | RPMVSF_NEEDPAYLOAD;
    rpmKeyring keyring = rpmtsGetKeyring(ts, 1);
    struct rpmvs_s *vs = rpmvsCreate(0, vsflags, keyring);
    struct pkgdata_s pkgdata = {
	.msgfunc = loghdrmsg,
	.fn = fn ? fn : Fdescr(fd),
	.msg = NULL,
	.rc = RPMRC_OK,
    };
    if (hdrp)
	*hdrp = NULL;
    rpmRC rc = rpmpkgRead(vs, fd, &sigblob, &blob, &msg);
    if (rc)
	goto exit;
    rc = RPMRC_FAIL;
    if (!rpmvsVerify(vs, RPMSIG_VERIFIABLE_TYPE, handleHdrVS, &pkgdata)) {
	if (hdrp) {
	    if (hdrblobImport(sigblob, 0, &sigh, &msg))
		goto exit;
	    if (hdrblobImport(blob, 0, &h, &msg))
		goto exit;
	    headerMergeLegacySigs(h, sigh);
	    applyRetrofits(h);
	    *hdrp = headerLink(h);
	}
	rc = RPMRC_OK;
    }
    if (rc == RPMRC_OK && pkgdata.rc)
	rc = pkgdata.rc;
exit:
    if (rc && msg)
	rpmlog(RPMLOG_ERR, "%s: %s\n", Fdescr(fd), msg);
    hdrblobFree(sigblob);
    hdrblobFree(blob);
    headerFree(sigh);
    headerFree(h);
    rpmKeyringFree(keyring);
    rpmvsFree(vs);
    free(msg);
    return rc;
}