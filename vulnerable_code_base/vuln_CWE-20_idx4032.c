static int ovl_remove_upper(struct dentry *dentry, bool is_dir)
{
	struct dentry *upperdir = ovl_dentry_upper(dentry->d_parent);
	struct inode *dir = upperdir->d_inode;
	struct dentry *upper = ovl_dentry_upper(dentry);
	int err;
	inode_lock_nested(dir, I_MUTEX_PARENT);
	err = -ESTALE;
	if (upper->d_parent == upperdir) {
		dget(upper);
		if (is_dir)
			err = vfs_rmdir(dir, upper);
		else
			err = vfs_unlink(dir, upper, NULL);
		dput(upper);
		ovl_dentry_version_inc(dentry->d_parent);
	}
	if (!err)
		d_drop(dentry);
	inode_unlock(dir);
	return err;
}