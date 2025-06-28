static int fsmMkdirs(rpmfiles files, rpmfs fs, rpmPlugins plugins)
{
    DNLI_t dnli = dnlInitIterator(files, fs, 0);
    struct stat sb;
    const char *dpath;
    int rc = 0;
    int i;
    size_t ldnlen = 0;
    const char * ldn = NULL;
    while ((dpath = dnlNextIterator(dnli)) != NULL) {
	size_t dnlen = strlen(dpath);
	char * te, dn[dnlen+1];
	if (dnlen <= 1)
	    continue;
	if (dnlen == ldnlen && rstreq(dpath, ldn))
	    continue;
	(void) stpcpy(dn, dpath);
	for (i = 1, te = dn + 1; *te != '\0'; te++, i++) {
	    if (*te != '/')
		continue;
	    if (i < ldnlen &&
		(ldn[i] == '/' || ldn[i] == '\0') && rstreqn(dn, ldn, i))
		continue;
	    *te = '\0';
	    rc = fsmStat(dn, 1, &sb);  
	    *te = '/';
	    if (rc == 0 && S_ISDIR(sb.st_mode)) {
		continue;
	    } else if (rc == RPMERR_ENOENT) {
		*te = '\0';
		mode_t mode = S_IFDIR | (_dirPerms & 07777);
		rpmFsmOp op = (FA_CREATE|FAF_UNOWNED);
		rc = rpmpluginsCallFsmFilePre(plugins, NULL, dn, mode, op);
		if (!rc)
		    rc = fsmMkdir(dn, mode);
		if (!rc) {
		    rc = rpmpluginsCallFsmFilePrepare(plugins, NULL, dn, dn,
						      mode, op);
		}
		rpmpluginsCallFsmFilePost(plugins, NULL, dn, mode, op, rc);
		if (!rc) {
		    rpmlog(RPMLOG_DEBUG,
			    "%s directory created with perms %04o\n",
			    dn, (unsigned)(mode & 07777));
		}
		*te = '/';
	    }
	    if (rc)
		break;
	}
	if (rc) break;
	ldn = dpath;
	ldnlen = dnlen;
    }
    dnlFreeIterator(dnli);
    return rc;
}