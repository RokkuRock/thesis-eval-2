int inode_change_ok(const struct inode *inode, struct iattr *attr)
{
	unsigned int ia_valid = attr->ia_valid;
	if (ia_valid & ATTR_SIZE) {
		int error = inode_newsize_ok(inode, attr->ia_size);
		if (error)
			return error;
	}
	if (ia_valid & ATTR_FORCE)
		return 0;
	if ((ia_valid & ATTR_UID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	     !uid_eq(attr->ia_uid, inode->i_uid)) &&
	    !inode_capable(inode, CAP_CHOWN))
		return -EPERM;
	if ((ia_valid & ATTR_GID) &&
	    (!uid_eq(current_fsuid(), inode->i_uid) ||
	    (!in_group_p(attr->ia_gid) && !gid_eq(attr->ia_gid, inode->i_gid))) &&
	    !inode_capable(inode, CAP_CHOWN))
		return -EPERM;
	if (ia_valid & ATTR_MODE) {
		if (!inode_owner_or_capable(inode))
			return -EPERM;
		if (!in_group_p((ia_valid & ATTR_GID) ? attr->ia_gid :
				inode->i_gid) &&
		    !inode_capable(inode, CAP_FSETID))
			attr->ia_mode &= ~S_ISGID;
	}
	if (ia_valid & (ATTR_MTIME_SET | ATTR_ATIME_SET | ATTR_TIMES_SET)) {
		if (!inode_owner_or_capable(inode))
			return -EPERM;
	}
	return 0;
}