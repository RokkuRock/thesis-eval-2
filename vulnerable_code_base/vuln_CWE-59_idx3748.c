mountpoint_last(struct nameidata *nd, struct path *path)
{
	int error = 0;
	struct dentry *dentry;
	struct dentry *dir = nd->path.dentry;
	if (nd->flags & LOOKUP_RCU) {
		if (unlazy_walk(nd, NULL)) {
			error = -ECHILD;
			goto out;
		}
	}
	nd->flags &= ~LOOKUP_PARENT;
	if (unlikely(nd->last_type != LAST_NORM)) {
		error = handle_dots(nd, nd->last_type);
		if (error)
			goto out;
		dentry = dget(nd->path.dentry);
		goto done;
	}
	mutex_lock(&dir->d_inode->i_mutex);
	dentry = d_lookup(dir, &nd->last);
	if (!dentry) {
		dentry = d_alloc(dir, &nd->last);
		if (!dentry) {
			error = -ENOMEM;
			mutex_unlock(&dir->d_inode->i_mutex);
			goto out;
		}
		dentry = lookup_real(dir->d_inode, dentry, nd->flags);
		error = PTR_ERR(dentry);
		if (IS_ERR(dentry)) {
			mutex_unlock(&dir->d_inode->i_mutex);
			goto out;
		}
	}
	mutex_unlock(&dir->d_inode->i_mutex);
done:
	if (!dentry->d_inode || d_is_negative(dentry)) {
		error = -ENOENT;
		dput(dentry);
		goto out;
	}
	path->dentry = dentry;
	path->mnt = mntget(nd->path.mnt);
	if (should_follow_link(dentry, nd->flags & LOOKUP_FOLLOW))
		return 1;
	follow_mount(path);
	error = 0;
out:
	terminate_walk(nd);
	return error;
}