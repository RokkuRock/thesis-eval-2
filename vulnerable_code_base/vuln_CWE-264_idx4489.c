int generic_permission(struct inode *inode, int mask)
{
	int ret;
	ret = acl_permission_check(inode, mask);
	if (ret != -EACCES)
		return ret;
	if (S_ISDIR(inode->i_mode)) {
		if (inode_capable(inode, CAP_DAC_OVERRIDE))
			return 0;
		if (!(mask & MAY_WRITE))
			if (inode_capable(inode, CAP_DAC_READ_SEARCH))
				return 0;
		return -EACCES;
	}
	if (!(mask & MAY_EXEC) || (inode->i_mode & S_IXUGO))
		if (inode_capable(inode, CAP_DAC_OVERRIDE))
			return 0;
	mask &= MAY_READ | MAY_WRITE | MAY_EXEC;
	if (mask == MAY_READ)
		if (inode_capable(inode, CAP_DAC_READ_SEARCH))
			return 0;
	return -EACCES;
}