xfs_iget_cache_miss(
	struct xfs_mount	*mp,
	struct xfs_perag	*pag,
	xfs_trans_t		*tp,
	xfs_ino_t		ino,
	struct xfs_inode	**ipp,
	int			flags,
	int			lock_flags)
{
	struct xfs_inode	*ip;
	int			error;
	xfs_agino_t		agino = XFS_INO_TO_AGINO(mp, ino);
	int			iflags;
	ip = xfs_inode_alloc(mp, ino);
	if (!ip)
		return -ENOMEM;
	error = xfs_iread(mp, tp, ip, flags);
	if (error)
		goto out_destroy;
	if (!xfs_inode_verify_forks(ip)) {
		error = -EFSCORRUPTED;
		goto out_destroy;
	}
	trace_xfs_iget_miss(ip);
	if (flags & XFS_IGET_CREATE) {
		if (VFS_I(ip)->i_mode != 0) {
			xfs_warn(mp,
"Corruption detected! Free inode 0x%llx not marked free on disk",
				ino);
			error = -EFSCORRUPTED;
			goto out_destroy;
		}
		if (ip->i_d.di_nblocks != 0) {
			xfs_warn(mp,
"Corruption detected! Free inode 0x%llx has blocks allocated!",
				ino);
			error = -EFSCORRUPTED;
			goto out_destroy;
		}
	} else if (VFS_I(ip)->i_mode == 0) {
		error = -ENOENT;
		goto out_destroy;
	}
	if (radix_tree_preload(GFP_NOFS)) {
		error = -EAGAIN;
		goto out_destroy;
	}
	if (lock_flags) {
		if (!xfs_ilock_nowait(ip, lock_flags))
			BUG();
	}
	iflags = XFS_INEW;
	if (flags & XFS_IGET_DONTCACHE)
		iflags |= XFS_IDONTCACHE;
	ip->i_udquot = NULL;
	ip->i_gdquot = NULL;
	ip->i_pdquot = NULL;
	xfs_iflags_set(ip, iflags);
	spin_lock(&pag->pag_ici_lock);
	error = radix_tree_insert(&pag->pag_ici_root, agino, ip);
	if (unlikely(error)) {
		WARN_ON(error != -EEXIST);
		XFS_STATS_INC(mp, xs_ig_dup);
		error = -EAGAIN;
		goto out_preload_end;
	}
	spin_unlock(&pag->pag_ici_lock);
	radix_tree_preload_end();
	*ipp = ip;
	return 0;
out_preload_end:
	spin_unlock(&pag->pag_ici_lock);
	radix_tree_preload_end();
	if (lock_flags)
		xfs_iunlock(ip, lock_flags);
out_destroy:
	__destroy_inode(VFS_I(ip));
	xfs_inode_free(ip);
	return error;
}