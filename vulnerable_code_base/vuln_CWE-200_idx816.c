static int ext4_write_end(struct file *file,
			  struct address_space *mapping,
			  loff_t pos, unsigned len, unsigned copied,
			  struct page *page, void *fsdata)
{
	handle_t *handle = ext4_journal_current_handle();
	struct inode *inode = mapping->host;
	loff_t old_size = inode->i_size;
	int ret = 0, ret2;
	int i_size_changed = 0;
	trace_ext4_write_end(inode, pos, len, copied);
	if (ext4_test_inode_state(inode, EXT4_STATE_ORDERED_MODE)) {
		ret = ext4_jbd2_file_inode(handle, inode);
		if (ret) {
			unlock_page(page);
			put_page(page);
			goto errout;
		}
	}
	if (ext4_has_inline_data(inode)) {
		ret = ext4_write_inline_data_end(inode, pos, len,
						 copied, page);
		if (ret < 0)
			goto errout;
		copied = ret;
	} else
		copied = block_write_end(file, mapping, pos,
					 len, copied, page, fsdata);
	i_size_changed = ext4_update_inode_size(inode, pos + copied);
	unlock_page(page);
	put_page(page);
	if (old_size < pos)
		pagecache_isize_extended(inode, old_size, pos);
	if (i_size_changed)
		ext4_mark_inode_dirty(handle, inode);
	if (pos + len > inode->i_size && ext4_can_truncate(inode))
		ext4_orphan_add(handle, inode);
errout:
	ret2 = ext4_journal_stop(handle);
	if (!ret)
		ret = ret2;
	if (pos + len > inode->i_size) {
		ext4_truncate_failed_write(inode);
		if (inode->i_nlink)
			ext4_orphan_del(NULL, inode);
	}
	return ret ? ret : copied;
}