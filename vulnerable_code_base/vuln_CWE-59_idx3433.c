int rpmPackageFilesInstall(rpmts ts, rpmte te, rpmfiles files,
              rpmpsm psm, char ** failedFile)
{
    FD_t payload = rpmtePayload(te);
    rpmfi fi = rpmfiNewArchiveReader(payload, files, RPMFI_ITER_READ_ARCHIVE);
    rpmfs fs = rpmteGetFileStates(te);
    rpmPlugins plugins = rpmtsPlugins(ts);
    struct stat sb;
    int saveerrno = errno;
    int rc = 0;
    int nodigest = (rpmtsFlags(ts) & RPMTRANS_FLAG_NOFILEDIGEST) ? 1 : 0;
    int nofcaps = (rpmtsFlags(ts) & RPMTRANS_FLAG_NOCAPS) ? 1 : 0;
    int firsthardlink = -1;
    int skip;
    rpmFileAction action;
    char *tid = NULL;
    const char *suffix;
    char *fpath = NULL;
    if (fi == NULL) {
	rc = RPMERR_BAD_MAGIC;
	goto exit;
    }
    rasprintf(&tid, ";%08x", (unsigned)rpmtsGetTid(ts));
    rc = fsmMkdirs(files, fs, plugins);
    while (!rc) {
	rc = rpmfiNext(fi);
	if (rc < 0) {
	    if (rc == RPMERR_ITER_END)
		rc = 0;
	    break;
	}
	action = rpmfsGetAction(fs, rpmfiFX(fi));
	skip = XFA_SKIPPING(action);
	suffix = S_ISDIR(rpmfiFMode(fi)) ? NULL : tid;
	if (action != FA_TOUCH) {
	    fpath = fsmFsPath(fi, suffix);
	} else {
	    fpath = fsmFsPath(fi, "");
	}
	rc = rpmfiStat(fi, 1, &sb);
	fsmDebug(fpath, action, &sb);
        if (rc)
            break;
	rc = rpmpluginsCallFsmFilePre(plugins, fi, fpath,
				      sb.st_mode, action);
	if (rc) {
	    skip = 1;
	} else {
	    setFileState(fs, rpmfiFX(fi));
	}
        if (!skip) {
	    int setmeta = 1;
	    if (!suffix) {
		rc = fsmBackup(fi, action);
	    }
	    if (!suffix) {
		rc = fsmVerify(fpath, fi);
	    } else {
		rc = (action == FA_TOUCH) ? 0 : RPMERR_ENOENT;
	    }
            if (S_ISREG(sb.st_mode)) {
		if (rc == RPMERR_ENOENT) {
		    rc = fsmMkfile(fi, fpath, files, psm, nodigest,
				   &setmeta, &firsthardlink);
		}
            } else if (S_ISDIR(sb.st_mode)) {
                if (rc == RPMERR_ENOENT) {
                    mode_t mode = sb.st_mode;
                    mode &= ~07777;
                    mode |=  00700;
                    rc = fsmMkdir(fpath, mode);
                }
            } else if (S_ISLNK(sb.st_mode)) {
		if (rc == RPMERR_ENOENT) {
		    rc = fsmSymlink(rpmfiFLink(fi), fpath);
		}
            } else if (S_ISFIFO(sb.st_mode)) {
                if (rc == RPMERR_ENOENT) {
                    rc = fsmMkfifo(fpath, 0000);
                }
            } else if (S_ISCHR(sb.st_mode) ||
                       S_ISBLK(sb.st_mode) ||
                       S_ISSOCK(sb.st_mode))
            {
                if (rc == RPMERR_ENOENT) {
                    rc = fsmMknod(fpath, sb.st_mode, sb.st_rdev);
                }
            } else {
                if (!IS_DEV_LOG(fpath))
                    rc = RPMERR_UNKNOWN_FILETYPE;
            }
	    if (!rc && setmeta) {
		rc = fsmSetmeta(fpath, fi, plugins, action, &sb, nofcaps);
	    }
        } else if (firsthardlink >= 0 && rpmfiArchiveHasContent(fi)) {
	    char *fn = rpmfilesFN(files, firsthardlink);
	    rc = expandRegular(fi, fn, psm, nodigest, 0);
	    firsthardlink = -1;
	    free(fn);
	}
        if (rc) {
            if (!skip) {
                if (suffix && (action != FA_TOUCH)) {
		    (void) fsmRemove(fpath, sb.st_mode);
                }
                errno = saveerrno;
            }
        } else {
	    rpmpsmNotify(psm, RPMCALLBACK_INST_PROGRESS, rpmfiArchiveTell(fi));
	    if (!skip) {
		if (suffix)
		    rc = fsmBackup(fi, action);
		if (!rc)
		    rc = fsmCommit(&fpath, fi, action, suffix);
	    }
	}
	if (rc)
	    *failedFile = xstrdup(fpath);
	rpmpluginsCallFsmFilePost(plugins, fi, fpath,
				  sb.st_mode, action, rc);
	fpath = _free(fpath);
    }
    rpmswAdd(rpmtsOp(ts, RPMTS_OP_UNCOMPRESS), fdOp(payload, FDSTAT_READ));
    rpmswAdd(rpmtsOp(ts, RPMTS_OP_DIGEST), fdOp(payload, FDSTAT_DIGEST));
exit:
    rpmfiArchiveClose(fi);
    rpmfiFree(fi);
    Fclose(payload);
    free(tid);
    free(fpath);
    return rc;
}