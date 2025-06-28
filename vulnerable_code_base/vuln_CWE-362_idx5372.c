static int do_setxattr(struct btrfs_trans_handle *trans,
		       struct inode *inode, const char *name,
		       const void *value, size_t size, int flags)
{
	struct btrfs_dir_item *di;
	struct btrfs_root *root = BTRFS_I(inode)->root;
	struct btrfs_path *path;
	size_t name_len = strlen(name);
	int ret = 0;
	if (name_len + size > BTRFS_MAX_XATTR_SIZE(root))
		return -ENOSPC;
	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;
	if (flags & XATTR_REPLACE) {
		di = btrfs_lookup_xattr(trans, root, path, btrfs_ino(inode), name,
					name_len, -1);
		if (IS_ERR(di)) {
			ret = PTR_ERR(di);
			goto out;
		} else if (!di) {
			ret = -ENODATA;
			goto out;
		}
		ret = btrfs_delete_one_dir_name(trans, root, path, di);
		if (ret)
			goto out;
		btrfs_release_path(path);
		if (!value)
			goto out;
	} else {
		di = btrfs_lookup_xattr(NULL, root, path, btrfs_ino(inode),
					name, name_len, 0);
		if (IS_ERR(di)) {
			ret = PTR_ERR(di);
			goto out;
		}
		if (!di && !value)
			goto out;
		btrfs_release_path(path);
	}
again:
	ret = btrfs_insert_xattr_item(trans, root, path, btrfs_ino(inode),
				      name, name_len, value, size);
	if (ret == -EOVERFLOW)
		ret = -EEXIST;
	if (ret == -EEXIST) {
		if (flags & XATTR_CREATE)
			goto out;
		btrfs_release_path(path);
		di = btrfs_lookup_xattr(trans, root, path, btrfs_ino(inode),
					name, name_len, -1);
		if (IS_ERR(di)) {
			ret = PTR_ERR(di);
			goto out;
		} else if (!di) {
			btrfs_release_path(path);
			goto again;
		}
		ret = btrfs_delete_one_dir_name(trans, root, path, di);
		if (ret)
			goto out;
		if (value) {
			btrfs_release_path(path);
			goto again;
		}
	}
out:
	btrfs_free_path(path);
	return ret;
}