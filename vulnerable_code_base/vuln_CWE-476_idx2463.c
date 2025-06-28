static int fscrypt_d_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct dentry *dir;
	struct fscrypt_info *ci;
	int dir_has_key, cached_with_key;
	if (flags & LOOKUP_RCU)
		return -ECHILD;
	dir = dget_parent(dentry);
	if (!d_inode(dir)->i_sb->s_cop->is_encrypted(d_inode(dir))) {
		dput(dir);
		return 0;
	}
	ci = d_inode(dir)->i_crypt_info;
	if (ci && ci->ci_keyring_key &&
	    (ci->ci_keyring_key->flags & ((1 << KEY_FLAG_INVALIDATED) |
					  (1 << KEY_FLAG_REVOKED) |
					  (1 << KEY_FLAG_DEAD))))
		ci = NULL;
	spin_lock(&dentry->d_lock);
	cached_with_key = dentry->d_flags & DCACHE_ENCRYPTED_WITH_KEY;
	spin_unlock(&dentry->d_lock);
	dir_has_key = (ci != NULL);
	dput(dir);
	if ((!cached_with_key && d_is_negative(dentry)) ||
			(!cached_with_key && dir_has_key) ||
			(cached_with_key && !dir_has_key))
		return 0;
	return 1;
}