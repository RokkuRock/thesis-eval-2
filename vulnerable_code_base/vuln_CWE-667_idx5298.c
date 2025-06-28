tdirenter(
	struct tmount	*tm,
	struct tmpnode	*dir,		 
	char		*name,		 
	enum de_op	op,		 
	struct tmpnode	*fromparent,	 
	struct tmpnode	*tp,		 
	struct vattr	*va,
	struct tmpnode	**tpp,		 
	struct cred	*cred,
	caller_context_t *ctp)
{
	struct tdirent *tdp;
	struct tmpnode *found = NULL;
	int error = 0;
	char *s;
	ASSERT(RW_WRITE_HELD(&dir->tn_rwlock));
	ASSERT(dir->tn_type == VDIR);
	for (s = name; *s; s++)
		if (*s == '/')
			return (EACCES);
	if (name[0] == '\0')
		panic("tdirenter: NULL name");
	if (op == DE_LINK || op == DE_RENAME) {
		if (tp != dir)
			rw_enter(&tp->tn_rwlock, RW_WRITER);
		mutex_enter(&tp->tn_tlock);
		if (tp->tn_nlink == 0) {
			mutex_exit(&tp->tn_tlock);
			if (tp != dir)
				rw_exit(&tp->tn_rwlock);
			return (ENOENT);
		}
		if (tp->tn_nlink == MAXLINK) {
			mutex_exit(&tp->tn_tlock);
			if (tp != dir)
				rw_exit(&tp->tn_rwlock);
			return (EMLINK);
		}
		tp->tn_nlink++;
		gethrestime(&tp->tn_ctime);
		mutex_exit(&tp->tn_tlock);
		if (tp != dir)
			rw_exit(&tp->tn_rwlock);
	}
	if (dir->tn_nlink == 0) {
		error = ENOENT;
		goto out;
	}
	if (op == DE_RENAME) {
		if (tp == dir) {
			error = EINVAL;
			goto out;
		}
		if (tp->tn_type == VDIR) {
			if ((fromparent != dir) &&
			    (error = tdircheckpath(tp, dir, cred))) {
				goto out;
			}
		}
	}
	tdp = tmpfs_hash_lookup(name, dir, 1, &found);
	if (tdp) {
		ASSERT(found);
		switch (op) {
		case DE_CREATE:
		case DE_MKDIR:
			if (tpp) {
				*tpp = found;
				error = EEXIST;
			} else {
				tmpnode_rele(found);
			}
			break;
		case DE_RENAME:
			error = tdirrename(fromparent, tp,
			    dir, name, found, tdp, cred);
			if (error == 0) {
				if (found != NULL) {
					vnevent_rename_dest(TNTOV(found),
					    TNTOV(dir), name, ctp);
				}
			}
			tmpnode_rele(found);
			break;
		case DE_LINK:
			error = EEXIST;
			tmpnode_rele(found);
			break;
		}
	} else {
		if (error = tmp_taccess(dir, VWRITE, cred))
			goto out;
		if (op == DE_CREATE || op == DE_MKDIR) {
			error = tdirmaketnode(dir, tm, va, op, &tp, cred);
			if (error)
				goto out;
		}
		if (error = tdiraddentry(dir, tp, name, op, fromparent)) {
			if (op == DE_CREATE || op == DE_MKDIR) {
				rw_enter(&tp->tn_rwlock, RW_WRITER);
				if ((tp->tn_type) == VDIR) {
					ASSERT(tdp == NULL);
					tdirtrunc(tp);
				}
				mutex_enter(&tp->tn_tlock);
				tp->tn_nlink = 0;
				mutex_exit(&tp->tn_tlock);
				gethrestime(&tp->tn_ctime);
				rw_exit(&tp->tn_rwlock);
				tmpnode_rele(tp);
				tp = NULL;
			}
		} else if (tpp) {
			*tpp = tp;
		} else if (op == DE_CREATE || op == DE_MKDIR) {
			tmpnode_rele(tp);
		}
	}
out:
	if (error && (op == DE_LINK || op == DE_RENAME)) {
		DECR_COUNT(&tp->tn_nlink, &tp->tn_tlock);
		gethrestime(&tp->tn_ctime);
	}
	return (error);
}