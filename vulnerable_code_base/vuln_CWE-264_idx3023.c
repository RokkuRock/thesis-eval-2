xfs_ioctl_setattr(
	xfs_inode_t		*ip,
	struct fsxattr		*fa,
	int			mask)
{
	struct xfs_mount	*mp = ip->i_mount;
	struct xfs_trans	*tp;
	unsigned int		lock_flags = 0;
	struct xfs_dquot	*udqp = NULL;
	struct xfs_dquot	*pdqp = NULL;
	struct xfs_dquot	*olddquot = NULL;
	int			code;
	trace_xfs_ioctl_setattr(ip);
	if (mp->m_flags & XFS_MOUNT_RDONLY)
		return XFS_ERROR(EROFS);
	if (XFS_FORCED_SHUTDOWN(mp))
		return XFS_ERROR(EIO);
	if ((mask & FSX_PROJID) && (fa->fsx_projid > (__uint16_t)-1) &&
			!xfs_sb_version_hasprojid32bit(&ip->i_mount->m_sb))
		return XFS_ERROR(EINVAL);
	if (XFS_IS_QUOTA_ON(mp) && (mask & FSX_PROJID)) {
		code = xfs_qm_vop_dqalloc(ip, ip->i_d.di_uid,
					 ip->i_d.di_gid, fa->fsx_projid,
					 XFS_QMOPT_PQUOTA, &udqp, NULL, &pdqp);
		if (code)
			return code;
	}
	tp = xfs_trans_alloc(mp, XFS_TRANS_SETATTR_NOT_SIZE);
	code = xfs_trans_reserve(tp, &M_RES(mp)->tr_ichange, 0, 0);
	if (code)
		goto error_return;
	lock_flags = XFS_ILOCK_EXCL;
	xfs_ilock(ip, lock_flags);
	if (!inode_owner_or_capable(VFS_I(ip))) {
		code = XFS_ERROR(EPERM);
		goto error_return;
	}
	if (mask & FSX_PROJID) {
		if (current_user_ns() != &init_user_ns) {
			code = XFS_ERROR(EINVAL);
			goto error_return;
		}
		if (XFS_IS_QUOTA_RUNNING(mp) &&
		    XFS_IS_PQUOTA_ON(mp) &&
		    xfs_get_projid(ip) != fa->fsx_projid) {
			ASSERT(tp);
			code = xfs_qm_vop_chown_reserve(tp, ip, udqp, NULL,
						pdqp, capable(CAP_FOWNER) ?
						XFS_QMOPT_FORCE_RES : 0);
			if (code)	 
				goto error_return;
		}
	}
	if (mask & FSX_EXTSIZE) {
		if (ip->i_d.di_nextents &&
		    ((ip->i_d.di_extsize << mp->m_sb.sb_blocklog) !=
		     fa->fsx_extsize)) {
			code = XFS_ERROR(EINVAL);	 
			goto error_return;
		}
		if (fa->fsx_extsize != 0) {
			xfs_extlen_t    size;
			xfs_fsblock_t   extsize_fsb;
			extsize_fsb = XFS_B_TO_FSB(mp, fa->fsx_extsize);
			if (extsize_fsb > MAXEXTLEN) {
				code = XFS_ERROR(EINVAL);
				goto error_return;
			}
			if (XFS_IS_REALTIME_INODE(ip) ||
			    ((mask & FSX_XFLAGS) &&
			    (fa->fsx_xflags & XFS_XFLAG_REALTIME))) {
				size = mp->m_sb.sb_rextsize <<
				       mp->m_sb.sb_blocklog;
			} else {
				size = mp->m_sb.sb_blocksize;
				if (extsize_fsb > mp->m_sb.sb_agblocks / 2) {
					code = XFS_ERROR(EINVAL);
					goto error_return;
				}
			}
			if (fa->fsx_extsize % size) {
				code = XFS_ERROR(EINVAL);
				goto error_return;
			}
		}
	}
	if (mask & FSX_XFLAGS) {
		if ((ip->i_d.di_nextents || ip->i_delayed_blks) &&
		    (XFS_IS_REALTIME_INODE(ip)) !=
		    (fa->fsx_xflags & XFS_XFLAG_REALTIME)) {
			code = XFS_ERROR(EINVAL);	 
			goto error_return;
		}
		if ((fa->fsx_xflags & XFS_XFLAG_REALTIME)) {
			if ((mp->m_sb.sb_rblocks == 0) ||
			    (mp->m_sb.sb_rextsize == 0) ||
			    (ip->i_d.di_extsize % mp->m_sb.sb_rextsize)) {
				code = XFS_ERROR(EINVAL);
				goto error_return;
			}
		}
		if ((ip->i_d.di_flags &
				(XFS_DIFLAG_IMMUTABLE|XFS_DIFLAG_APPEND) ||
		     (fa->fsx_xflags &
				(XFS_XFLAG_IMMUTABLE | XFS_XFLAG_APPEND))) &&
		    !capable(CAP_LINUX_IMMUTABLE)) {
			code = XFS_ERROR(EPERM);
			goto error_return;
		}
	}
	xfs_trans_ijoin(tp, ip, 0);
	if (mask & FSX_PROJID) {
		if ((ip->i_d.di_mode & (S_ISUID|S_ISGID)) &&
		    !inode_capable(VFS_I(ip), CAP_FSETID))
			ip->i_d.di_mode &= ~(S_ISUID|S_ISGID);
		if (xfs_get_projid(ip) != fa->fsx_projid) {
			if (XFS_IS_QUOTA_RUNNING(mp) && XFS_IS_PQUOTA_ON(mp)) {
				olddquot = xfs_qm_vop_chown(tp, ip,
							&ip->i_pdquot, pdqp);
			}
			xfs_set_projid(ip, fa->fsx_projid);
			if (ip->i_d.di_version == 1)
				xfs_bump_ino_vers2(tp, ip);
		}
	}
	if (mask & FSX_EXTSIZE)
		ip->i_d.di_extsize = fa->fsx_extsize >> mp->m_sb.sb_blocklog;
	if (mask & FSX_XFLAGS) {
		xfs_set_diflags(ip, fa->fsx_xflags);
		xfs_diflags_to_linux(ip);
	}
	xfs_trans_ichgtime(tp, ip, XFS_ICHGTIME_CHG);
	xfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
	XFS_STATS_INC(xs_ig_attrchg);
	if (mp->m_flags & XFS_MOUNT_WSYNC)
		xfs_trans_set_sync(tp);
	code = xfs_trans_commit(tp, 0);
	xfs_iunlock(ip, lock_flags);
	xfs_qm_dqrele(olddquot);
	xfs_qm_dqrele(udqp);
	xfs_qm_dqrele(pdqp);
	return code;
 error_return:
	xfs_qm_dqrele(udqp);
	xfs_qm_dqrele(pdqp);
	xfs_trans_cancel(tp, 0);
	if (lock_flags)
		xfs_iunlock(ip, lock_flags);
	return code;
}