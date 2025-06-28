int rpmPackageFilesInstall(rpmts ts, rpmte te, rpmfiles files,
              rpmpsm psm, char ** failedFile)
{
    FD_t payload = rpmtePayload(te);
    rpmfi fi = NULL;
    rpmfs fs = rpmteGetFileStates(te);
    rpmPlugins plugins = rpmtsPlugins(ts);
    int rc = 0;
    int fx = -1;
    int fc = rpmfilesFC(files);
    int nodigest = (rpmtsFlags(ts) & RPMTRANS_FLAG_NOFILEDIGEST) ? 1 : 0;
    int nofcaps = (rpmtsFlags(ts) & RPMTRANS_FLAG_NOCAPS) ? 1 : 0;
    FD_t firstlinkfile = NULL;
    char *tid = NULL;
    struct filedata_s *fdata = xcalloc(fc, sizeof(*fdata));
    struct filedata_s *firstlink = NULL;
    rasprintf(&tid, ";%08x", (unsigned)rpmtsGetTid(ts));
    fi = rpmfilesIter(files, RPMFI_ITER_FWD);
    while (!rc && (fx = rpmfiNext(fi)) >= 0) {
	struct filedata_s *fp = &fdata[fx];
	if (rpmfiFFlags(fi) & RPMFILE_GHOST)
            fp->action = FA_SKIP;
	else
	    fp->action = rpmfsGetAction(fs, fx);
	fp->skip = XFA_SKIPPING(fp->action);
	fp->setmeta = 1;
	if (XFA_CREATING(fp->action) && !S_ISDIR(rpmfiFMode(fi)))
	    fp->suffix = tid;
	fp->fpath = fsmFsPath(fi, fp->suffix);
	rc = rpmfiStat(fi, 1, &fp->sb);
	setFileState(fs, fx);
	fsmDebug(fp->fpath, fp->action, &fp->sb);
	rc = rpmpluginsCallFsmFilePre(plugins, fi, fp->fpath,
				      fp->sb.st_mode, fp->action);
	fp->stage = FILE_PRE;
    }
    fi = rpmfiFree(fi);
    if (rc)
	goto exit;
    if (rpmteType(te) == TR_ADDED)
	fi = rpmfiNewArchiveReader(payload, files, RPMFI_ITER_READ_ARCHIVE);
    else
	fi = rpmfilesIter(files, RPMFI_ITER_FWD);
    if (fi == NULL) {
        rc = RPMERR_BAD_MAGIC;
        goto exit;
    }
    if (!rc)
	rc = fsmMkdirs(files, fs, plugins);
    while (!rc && (fx = rpmfiNext(fi)) >= 0) {
	struct filedata_s *fp = &fdata[fx];
        if (!fp->skip) {
	    if (!fp->suffix) {
		rc = fsmBackup(fi, fp->action);
	    }
	    if (!fp->suffix) {
		if (fp->action == FA_TOUCH) {
		    struct stat sb;
		    rc = fsmStat(fp->fpath, 1, &sb);
		} else {
		    rc = fsmVerify(fp->fpath, fi);
		}
	    } else {
		rc = RPMERR_ENOENT;
	    }
	    if (rc == RPMERR_ENOENT && fp->action == FA_TOUCH) {
		rpmlog(RPMLOG_DEBUG, "file %s vanished unexpectedly\n",
			fp->fpath);
		fp->action = FA_CREATE;
		fsmDebug(fp->fpath, fp->action, &fp->sb);
	    }
	    if (fp->action == FA_TOUCH)
		continue;
            if (S_ISREG(fp->sb.st_mode)) {
		if (rc == RPMERR_ENOENT) {
		    rc = fsmMkfile(fi, fp, files, psm, nodigest,
				   &firstlink, &firstlinkfile);
		}
            } else if (S_ISDIR(fp->sb.st_mode)) {
                if (rc == RPMERR_ENOENT) {
                    mode_t mode = fp->sb.st_mode;
                    mode &= ~07777;
                    mode |=  00700;
                    rc = fsmMkdir(fp->fpath, mode);
                }
            } else if (S_ISLNK(fp->sb.st_mode)) {
		if (rc == RPMERR_ENOENT) {
		    rc = fsmSymlink(rpmfiFLink(fi), fp->fpath);
		}
            } else if (S_ISFIFO(fp->sb.st_mode)) {
                if (rc == RPMERR_ENOENT) {
                    rc = fsmMkfifo(fp->fpath, 0000);
                }
            } else if (S_ISCHR(fp->sb.st_mode) ||
                       S_ISBLK(fp->sb.st_mode) ||
                       S_ISSOCK(fp->sb.st_mode))
            {
                if (rc == RPMERR_ENOENT) {
                    rc = fsmMknod(fp->fpath, fp->sb.st_mode, fp->sb.st_rdev);
                }
            } else {
                if (!IS_DEV_LOG(fp->fpath))
                    rc = RPMERR_UNKNOWN_FILETYPE;
            }
	} else if (firstlink && rpmfiArchiveHasContent(fi)) {
	    rc = fsmMkfile(fi, firstlink, files, psm, nodigest,
			   &firstlink, &firstlinkfile);
	}
	if (rc)
	    *failedFile = xstrdup(fp->fpath);
	else
	    rpmpsmNotify(psm, RPMCALLBACK_INST_PROGRESS, rpmfiArchiveTell(fi));
	fp->stage = FILE_UNPACK;
    }
    fi = rpmfiFree(fi);
    if (!rc && fx < 0 && fx != RPMERR_ITER_END)
	rc = fx;
    fi = rpmfilesIter(files, RPMFI_ITER_FWD);
    while (!rc && (fx = rpmfiNext(fi)) >= 0) {
	struct filedata_s *fp = &fdata[fx];
	if (!fp->skip && fp->setmeta) {
	    rc = fsmSetmeta(fp->fpath, fi, plugins, fp->action,
			    &fp->sb, nofcaps);
	}
	if (rc)
	    *failedFile = xstrdup(fp->fpath);
	fp->stage = FILE_PREP;
    }
    fi = rpmfiFree(fi);
    fi = rpmfilesIter(files, RPMFI_ITER_FWD);
    while (!rc && (fx = rpmfiNext(fi)) >= 0) {
	struct filedata_s *fp = &fdata[fx];
	if (!fp->skip) {
	    if (!rc && fp->suffix)
		rc = fsmBackup(fi, fp->action);
	    if (!rc)
		rc = fsmCommit(&fp->fpath, fi, fp->action, fp->suffix);
	    if (!rc)
		fp->stage = FILE_COMMIT;
	    else
		*failedFile = xstrdup(fp->fpath);
	}
    }
    fi = rpmfiFree(fi);
    fi = rpmfilesIter(files, RPMFI_ITER_BACK);
    while ((fx = rpmfiNext(fi)) >= 0) {
	struct filedata_s *fp = &fdata[fx];
	if (fp->stage) {
	    rpmpluginsCallFsmFilePost(plugins, fi, fp->fpath,
				      fp->sb.st_mode, fp->action, rc);
	}
	if (rc && fp->stage > FILE_NONE && !fp->skip) {
	    (void) fsmRemove(fp->fpath, fp->sb.st_mode);
	}
    }
    rpmswAdd(rpmtsOp(ts, RPMTS_OP_UNCOMPRESS), fdOp(payload, FDSTAT_READ));
    rpmswAdd(rpmtsOp(ts, RPMTS_OP_DIGEST), fdOp(payload, FDSTAT_DIGEST));
exit:
    fi = rpmfiFree(fi);
    Fclose(payload);
    free(tid);
    for (int i = 0; i < fc; i++)
	free(fdata[i].fpath);
    free(fdata);
    return rc;
}