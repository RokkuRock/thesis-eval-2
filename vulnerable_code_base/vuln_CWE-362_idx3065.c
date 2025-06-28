int ext4_insert_range(struct inode *inode, loff_t offset, loff_t len)
{
	struct super_block *sb = inode->i_sb;
	handle_t *handle;
	struct ext4_ext_path *path;
	struct ext4_extent *extent;
	ext4_lblk_t offset_lblk, len_lblk, ee_start_lblk = 0;
	unsigned int credits, ee_len;
	int ret = 0, depth, split_flag = 0;
	loff_t ioffset;
	if (!ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS))
		return -EOPNOTSUPP;
	if (offset & (EXT4_CLUSTER_SIZE(sb) - 1) ||
			len & (EXT4_CLUSTER_SIZE(sb) - 1))
		return -EINVAL;
	if (!S_ISREG(inode->i_mode))
		return -EOPNOTSUPP;
	trace_ext4_insert_range(inode, offset, len);
	offset_lblk = offset >> EXT4_BLOCK_SIZE_BITS(sb);
	len_lblk = len >> EXT4_BLOCK_SIZE_BITS(sb);
	if (ext4_should_journal_data(inode)) {
		ret = ext4_force_commit(inode->i_sb);
		if (ret)
			return ret;
	}
	ioffset = round_down(offset, PAGE_SIZE);
	ret = filemap_write_and_wait_range(inode->i_mapping, ioffset,
			LLONG_MAX);
	if (ret)
		return ret;
	mutex_lock(&inode->i_mutex);
	if (!ext4_test_inode_flag(inode, EXT4_INODE_EXTENTS)) {
		ret = -EOPNOTSUPP;
		goto out_mutex;
	}
	if (inode->i_size + len > inode->i_sb->s_maxbytes) {
		ret = -EFBIG;
		goto out_mutex;
	}
	if (offset >= i_size_read(inode)) {
		ret = -EINVAL;
		goto out_mutex;
	}
	truncate_pagecache(inode, ioffset);
	ext4_inode_block_unlocked_dio(inode);
	inode_dio_wait(inode);
	credits = ext4_writepage_trans_blocks(inode);
	handle = ext4_journal_start(inode, EXT4_HT_TRUNCATE, credits);
	if (IS_ERR(handle)) {
		ret = PTR_ERR(handle);
		goto out_dio;
	}
	inode->i_size += len;
	EXT4_I(inode)->i_disksize += len;
	inode->i_mtime = inode->i_ctime = ext4_current_time(inode);
	ret = ext4_mark_inode_dirty(handle, inode);
	if (ret)
		goto out_stop;
	down_write(&EXT4_I(inode)->i_data_sem);
	ext4_discard_preallocations(inode);
	path = ext4_find_extent(inode, offset_lblk, NULL, 0);
	if (IS_ERR(path)) {
		up_write(&EXT4_I(inode)->i_data_sem);
		goto out_stop;
	}
	depth = ext_depth(inode);
	extent = path[depth].p_ext;
	if (extent) {
		ee_start_lblk = le32_to_cpu(extent->ee_block);
		ee_len = ext4_ext_get_actual_len(extent);
		if ((offset_lblk > ee_start_lblk) &&
				(offset_lblk < (ee_start_lblk + ee_len))) {
			if (ext4_ext_is_unwritten(extent))
				split_flag = EXT4_EXT_MARK_UNWRIT1 |
					EXT4_EXT_MARK_UNWRIT2;
			ret = ext4_split_extent_at(handle, inode, &path,
					offset_lblk, split_flag,
					EXT4_EX_NOCACHE |
					EXT4_GET_BLOCKS_PRE_IO |
					EXT4_GET_BLOCKS_METADATA_NOFAIL);
		}
		ext4_ext_drop_refs(path);
		kfree(path);
		if (ret < 0) {
			up_write(&EXT4_I(inode)->i_data_sem);
			goto out_stop;
		}
	}
	ret = ext4_es_remove_extent(inode, offset_lblk,
			EXT_MAX_BLOCKS - offset_lblk);
	if (ret) {
		up_write(&EXT4_I(inode)->i_data_sem);
		goto out_stop;
	}
	ret = ext4_ext_shift_extents(inode, handle,
		ee_start_lblk > offset_lblk ? ee_start_lblk : offset_lblk,
		len_lblk, SHIFT_RIGHT);
	up_write(&EXT4_I(inode)->i_data_sem);
	if (IS_SYNC(inode))
		ext4_handle_sync(handle);
out_stop:
	ext4_journal_stop(handle);
out_dio:
	ext4_inode_resume_unlocked_dio(inode);
out_mutex:
	mutex_unlock(&inode->i_mutex);
	return ret;
}