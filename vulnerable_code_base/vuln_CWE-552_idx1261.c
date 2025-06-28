static int mnt_parse_mountinfo_line(struct libmnt_fs *fs, const char *s)
{
	int rc = 0;
	unsigned int maj, min;
	char *p;
	fs->flags |= MNT_FS_KERNEL;
	s = next_s32(s, &fs->id, &rc);
	if (!s || !*s || rc) {
		DBG(TAB, ul_debug("tab parse error: [id]"));
		goto fail;
	}
	s = skip_separator(s);
	s = next_s32(s, &fs->parent, &rc);
	if (!s || !*s || rc) {
		DBG(TAB, ul_debug("tab parse error: [parent]"));
		goto fail;
	}
	s = skip_separator(s);
	if (sscanf(s, "%u:%u", &maj, &min) != 2) {
		DBG(TAB, ul_debug("tab parse error: [maj:min]"));
		goto fail;
	}
	fs->devno = makedev(maj, min);
	s = skip_nonspearator(s);
	s = skip_separator(s);
	fs->root = unmangle(s, &s);
	if (!fs->root) {
		DBG(TAB, ul_debug("tab parse error: [mountroot]"));
		goto fail;
	}
	s = skip_separator(s);
	fs->target = unmangle(s, &s);
	if (!fs->target) {
		DBG(TAB, ul_debug("tab parse error: [target]"));
		goto fail;
	}
	p = (char *) endswith(fs->target, PATH_DELETED_SUFFIX);
	if (p && *p) {
		*p = '\0';
		fs->flags |= MNT_FS_DELETED;
	}
	s = skip_separator(s);
	fs->vfs_optstr = unmangle(s, &s);
	if (!fs->vfs_optstr) {
		DBG(TAB, ul_debug("tab parse error: [VFS options]"));
		goto fail;
	}
	p = strstr(s, " - ");
	if (!p) {
		DBG(TAB, ul_debug("mountinfo parse error: separator not found"));
		return -EINVAL;
	}
	if (p > s + 1)
		fs->opt_fields = strndup(s + 1, p - s - 1);
	s = skip_separator(p + 3);
	p = unmangle(s, &s);
	if (!p || (rc = __mnt_fs_set_fstype_ptr(fs, p))) {
		DBG(TAB, ul_debug("tab parse error: [fstype]"));
		free(p);
		goto fail;
	}
	if (!s || !*s) {
		DBG(TAB, ul_debug("tab parse error: [source]"));
		goto fail;
	} else if (*s == ' ' && *(s+1) == ' ') {
		if ((rc = mnt_fs_set_source(fs, ""))) {
			DBG(TAB, ul_debug("tab parse error: [empty source]"));
			goto fail;
		}
	} else {
		s = skip_separator(s);
		p = unmangle(s, &s);
		if (!p || (rc = __mnt_fs_set_source_ptr(fs, p))) {
			DBG(TAB, ul_debug("tab parse error: [regular source]"));
			free(p);
			goto fail;
		}
	}
	s = skip_separator(s);
	fs->fs_optstr = unmangle(s, &s);
	if (!fs->fs_optstr) {
		DBG(TAB, ul_debug("tab parse error: [FS options]"));
		goto fail;
	}
	fs->optstr = mnt_fs_strdup_options(fs);
	if (!fs->optstr) {
		rc = -ENOMEM;
		DBG(TAB, ul_debug("tab parse error: [merge VFS and FS options]"));
		goto fail;
	}
	return 0;
fail:
	if (rc == 0)
		rc = -EINVAL;
	DBG(TAB, ul_debug("tab parse error on: '%s' [rc=%d]", s, rc));
	return rc;
}