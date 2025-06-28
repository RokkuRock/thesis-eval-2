ext4_xattr_release_block(handle_t *handle, struct inode *inode,
			 struct buffer_head *bh)
{
	struct mb_cache_entry *ce = NULL;
	int error = 0;
	struct mb_cache *ext4_mb_cache = EXT4_GET_MB_CACHE(inode);
	ce = mb_cache_entry_get(ext4_mb_cache, bh->b_bdev, bh->b_blocknr);
	BUFFER_TRACE(bh, "get_write_access");
	error = ext4_journal_get_write_access(handle, bh);
	if (error)
		goto out;
	lock_buffer(bh);
	if (BHDR(bh)->h_refcount == cpu_to_le32(1)) {
		ea_bdebug(bh, "refcount now=0; freeing");
		if (ce)
			mb_cache_entry_free(ce);
		get_bh(bh);
		unlock_buffer(bh);
		ext4_free_blocks(handle, inode, bh, 0, 1,
				 EXT4_FREE_BLOCKS_METADATA |
				 EXT4_FREE_BLOCKS_FORGET);
	} else {
		le32_add_cpu(&BHDR(bh)->h_refcount, -1);
		if (ce)
			mb_cache_entry_release(ce);
		if (ext4_handle_valid(handle))
			error = ext4_handle_dirty_xattr_block(handle, inode,
							      bh);
		unlock_buffer(bh);
		if (!ext4_handle_valid(handle))
			error = ext4_handle_dirty_xattr_block(handle, inode,
							      bh);
		if (IS_SYNC(inode))
			ext4_handle_sync(handle);
		dquot_free_block(inode, EXT4_C2B(EXT4_SB(inode->i_sb), 1));
		ea_bdebug(bh, "refcount now=%d; releasing",
			  le32_to_cpu(BHDR(bh)->h_refcount));
	}
out:
	ext4_std_error(inode->i_sb, error);
	return;
}