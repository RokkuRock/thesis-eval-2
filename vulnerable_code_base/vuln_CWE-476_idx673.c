	int			lock_flags) __releases(RCU)
{
	struct inode		*inode = VFS_I(ip);
	struct xfs_mount	*mp = ip->i_mount;
	int			error;
	spin_lock(&ip->i_flags_lock);
	if (ip->i_ino != ino) {
		trace_xfs_iget_skip(ip);
		XFS_STATS_INC(mp, xs_ig_frecycle);
		error = -EAGAIN;
		goto out_error;
	}
	if (ip->i_flags & (XFS_INEW|XFS_IRECLAIM)) {
		trace_xfs_iget_skip(ip);
		XFS_STATS_INC(mp, xs_ig_frecycle);
		error = -EAGAIN;
		goto out_error;
	}
	if (VFS_I(ip)->i_mode == 0 && !(flags & XFS_IGET_CREATE)) {
		error = -ENOENT;
		goto out_error;
	}
	if (ip->i_flags & XFS_IRECLAIMABLE) {
		trace_xfs_iget_reclaim(ip);
		if (flags & XFS_IGET_INCORE) {
			error = -EAGAIN;
			goto out_error;
		}
		ip->i_flags |= XFS_IRECLAIM;
		spin_unlock(&ip->i_flags_lock);
		rcu_read_unlock();
		error = xfs_reinit_inode(mp, inode);
		if (error) {
			bool wake;
			rcu_read_lock();
			spin_lock(&ip->i_flags_lock);
			wake = !!__xfs_iflags_test(ip, XFS_INEW);
			ip->i_flags &= ~(XFS_INEW | XFS_IRECLAIM);
			if (wake)
				wake_up_bit(&ip->i_flags, __XFS_INEW_BIT);
			ASSERT(ip->i_flags & XFS_IRECLAIMABLE);
			trace_xfs_iget_reclaim_fail(ip);
			goto out_error;
		}
		spin_lock(&pag->pag_ici_lock);
		spin_lock(&ip->i_flags_lock);
		ip->i_flags &= ~XFS_IRECLAIM_RESET_FLAGS;
		ip->i_flags |= XFS_INEW;
		xfs_inode_clear_reclaim_tag(pag, ip->i_ino);
		inode->i_state = I_NEW;
		ASSERT(!rwsem_is_locked(&inode->i_rwsem));
		init_rwsem(&inode->i_rwsem);
		spin_unlock(&ip->i_flags_lock);
		spin_unlock(&pag->pag_ici_lock);
	} else {
		if (!igrab(inode)) {
			trace_xfs_iget_skip(ip);
			error = -EAGAIN;
			goto out_error;
		}
		spin_unlock(&ip->i_flags_lock);
		rcu_read_unlock();
		trace_xfs_iget_hit(ip);
	}
	if (lock_flags != 0)
		xfs_ilock(ip, lock_flags);
	if (!(flags & XFS_IGET_INCORE))
		xfs_iflags_clear(ip, XFS_ISTALE | XFS_IDONTCACHE);
	XFS_STATS_INC(mp, xs_ig_found);
	return 0;
out_error:
	spin_unlock(&ip->i_flags_lock);
	rcu_read_unlock();
	return error;
}