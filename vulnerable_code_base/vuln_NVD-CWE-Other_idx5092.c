int ext4_ext_get_blocks(handle_t *handle, struct inode *inode,
			ext4_lblk_t iblock,
			unsigned int max_blocks, struct buffer_head *bh_result,
			int flags)
{
	struct ext4_ext_path *path = NULL;
	struct ext4_extent_header *eh;
	struct ext4_extent newex, *ex, *last_ex;
	ext4_fsblk_t newblock;
	int err = 0, depth, ret, cache_type;
	unsigned int allocated = 0;
	struct ext4_allocation_request ar;
	ext4_io_end_t *io = EXT4_I(inode)->cur_aio_dio;
	__clear_bit(BH_New, &bh_result->b_state);
	ext_debug("blocks %u/%u requested for inode %lu\n",
			iblock, max_blocks, inode->i_ino);
	cache_type = ext4_ext_in_cache(inode, iblock, &newex);
	if (cache_type) {
		if (cache_type == EXT4_EXT_CACHE_GAP) {
			if ((flags & EXT4_GET_BLOCKS_CREATE) == 0) {
				goto out2;
			}
		} else if (cache_type == EXT4_EXT_CACHE_EXTENT) {
			newblock = iblock
				   - le32_to_cpu(newex.ee_block)
				   + ext_pblock(&newex);
			allocated = ext4_ext_get_actual_len(&newex) -
					(iblock - le32_to_cpu(newex.ee_block));
			goto out;
		} else {
			BUG();
		}
	}
	path = ext4_ext_find_extent(inode, iblock, NULL);
	if (IS_ERR(path)) {
		err = PTR_ERR(path);
		path = NULL;
		goto out2;
	}
	depth = ext_depth(inode);
	if (path[depth].p_ext == NULL && depth != 0) {
		ext4_error(inode->i_sb, "bad extent address "
			   "inode: %lu, iblock: %d, depth: %d",
			   inode->i_ino, iblock, depth);
		err = -EIO;
		goto out2;
	}
	eh = path[depth].p_hdr;
	ex = path[depth].p_ext;
	if (ex) {
		ext4_lblk_t ee_block = le32_to_cpu(ex->ee_block);
		ext4_fsblk_t ee_start = ext_pblock(ex);
		unsigned short ee_len;
		ee_len = ext4_ext_get_actual_len(ex);
		if (iblock >= ee_block && iblock < ee_block + ee_len) {
			newblock = iblock - ee_block + ee_start;
			allocated = ee_len - (iblock - ee_block);
			ext_debug("%u fit into %u:%d -> %llu\n", iblock,
					ee_block, ee_len, newblock);
			if (!ext4_ext_is_uninitialized(ex)) {
				ext4_ext_put_in_cache(inode, ee_block,
							ee_len, ee_start,
							EXT4_EXT_CACHE_EXTENT);
				goto out;
			}
			ret = ext4_ext_handle_uninitialized_extents(handle,
					inode, iblock, max_blocks, path,
					flags, allocated, bh_result, newblock);
			return ret;
		}
	}
	if ((flags & EXT4_GET_BLOCKS_CREATE) == 0) {
		ext4_ext_put_gap_in_cache(inode, path, iblock);
		goto out2;
	}
	ar.lleft = iblock;
	err = ext4_ext_search_left(inode, path, &ar.lleft, &ar.pleft);
	if (err)
		goto out2;
	ar.lright = iblock;
	err = ext4_ext_search_right(inode, path, &ar.lright, &ar.pright);
	if (err)
		goto out2;
	if (max_blocks > EXT_INIT_MAX_LEN &&
	    !(flags & EXT4_GET_BLOCKS_UNINIT_EXT))
		max_blocks = EXT_INIT_MAX_LEN;
	else if (max_blocks > EXT_UNINIT_MAX_LEN &&
		 (flags & EXT4_GET_BLOCKS_UNINIT_EXT))
		max_blocks = EXT_UNINIT_MAX_LEN;
	newex.ee_block = cpu_to_le32(iblock);
	newex.ee_len = cpu_to_le16(max_blocks);
	err = ext4_ext_check_overlap(inode, &newex, path);
	if (err)
		allocated = ext4_ext_get_actual_len(&newex);
	else
		allocated = max_blocks;
	ar.inode = inode;
	ar.goal = ext4_ext_find_goal(inode, path, iblock);
	ar.logical = iblock;
	ar.len = allocated;
	if (S_ISREG(inode->i_mode))
		ar.flags = EXT4_MB_HINT_DATA;
	else
		ar.flags = 0;
	newblock = ext4_mb_new_blocks(handle, &ar, &err);
	if (!newblock)
		goto out2;
	ext_debug("allocate new block: goal %llu, found %llu/%u\n",
		  ar.goal, newblock, allocated);
	ext4_ext_store_pblock(&newex, newblock);
	newex.ee_len = cpu_to_le16(ar.len);
	if (flags & EXT4_GET_BLOCKS_UNINIT_EXT){
		ext4_ext_mark_uninitialized(&newex);
		if (flags == EXT4_GET_BLOCKS_PRE_IO) {
			if (io)
				io->flag = EXT4_IO_UNWRITTEN;
			else
				ext4_set_inode_state(inode,
						     EXT4_STATE_DIO_UNWRITTEN);
		}
	}
	if (unlikely(EXT4_I(inode)->i_flags & EXT4_EOFBLOCKS_FL)) {
		if (eh->eh_entries) {
			last_ex = EXT_LAST_EXTENT(eh);
			if (iblock + ar.len > le32_to_cpu(last_ex->ee_block)
					    + ext4_ext_get_actual_len(last_ex))
				EXT4_I(inode)->i_flags &= ~EXT4_EOFBLOCKS_FL;
		} else {
			WARN_ON(eh->eh_entries == 0);
			ext4_error(inode->i_sb, __func__,
				"inode#%lu, eh->eh_entries = 0!", inode->i_ino);
			}
	}
	err = ext4_ext_insert_extent(handle, inode, path, &newex, flags);
	if (err) {
		ext4_discard_preallocations(inode);
		ext4_free_blocks(handle, inode, 0, ext_pblock(&newex),
				 ext4_ext_get_actual_len(&newex), 0);
		goto out2;
	}
	newblock = ext_pblock(&newex);
	allocated = ext4_ext_get_actual_len(&newex);
	if (allocated > max_blocks)
		allocated = max_blocks;
	set_buffer_new(bh_result);
	if (flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE)
		ext4_da_update_reserve_space(inode, allocated, 1);
	if ((flags & EXT4_GET_BLOCKS_UNINIT_EXT) == 0) {
		ext4_ext_put_in_cache(inode, iblock, allocated, newblock,
						EXT4_EXT_CACHE_EXTENT);
		ext4_update_inode_fsync_trans(handle, inode, 1);
	} else
		ext4_update_inode_fsync_trans(handle, inode, 0);
out:
	if (allocated > max_blocks)
		allocated = max_blocks;
	ext4_ext_show_leaf(inode, path);
	set_buffer_mapped(bh_result);
	bh_result->b_bdev = inode->i_sb->s_bdev;
	bh_result->b_blocknr = newblock;
out2:
	if (path) {
		ext4_ext_drop_refs(path);
		kfree(path);
	}
	return err ? err : allocated;
}