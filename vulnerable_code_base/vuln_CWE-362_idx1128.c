ext4_ext_handle_uninitialized_extents(handle_t *handle, struct inode *inode,
			struct ext4_map_blocks *map,
			struct ext4_ext_path *path, int flags,
			unsigned int allocated, ext4_fsblk_t newblock)
{
	int ret = 0;
	int err = 0;
	ext4_io_end_t *io = ext4_inode_aio(inode);
	ext_debug("ext4_ext_handle_uninitialized_extents: inode %lu, logical "
		  "block %llu, max_blocks %u, flags %x, allocated %u\n",
		  inode->i_ino, (unsigned long long)map->m_lblk, map->m_len,
		  flags, allocated);
	ext4_ext_show_leaf(inode, path);
	trace_ext4_ext_handle_uninitialized_extents(inode, map, allocated,
						    newblock);
	if ((flags & EXT4_GET_BLOCKS_PRE_IO)) {
		ret = ext4_split_unwritten_extents(handle, inode, map,
						   path, flags);
		if (ret <= 0)
			goto out;
		if (io)
			ext4_set_io_unwritten_flag(inode, io);
		else
			ext4_set_inode_state(inode, EXT4_STATE_DIO_UNWRITTEN);
		if (ext4_should_dioread_nolock(inode))
			map->m_flags |= EXT4_MAP_UNINIT;
		goto out;
	}
	if ((flags & EXT4_GET_BLOCKS_CONVERT)) {
		ret = ext4_convert_unwritten_extents_endio(handle, inode,
							path);
		if (ret >= 0) {
			ext4_update_inode_fsync_trans(handle, inode, 1);
			err = check_eofblocks_fl(handle, inode, map->m_lblk,
						 path, map->m_len);
		} else
			err = ret;
		goto out2;
	}
	if (flags & EXT4_GET_BLOCKS_UNINIT_EXT)
		goto map_out;
	if ((flags & EXT4_GET_BLOCKS_CREATE) == 0) {
		map->m_flags |= EXT4_MAP_UNWRITTEN;
		goto out1;
	}
	ret = ext4_ext_convert_to_initialized(handle, inode, map, path);
	if (ret >= 0)
		ext4_update_inode_fsync_trans(handle, inode, 1);
out:
	if (ret <= 0) {
		err = ret;
		goto out2;
	} else
		allocated = ret;
	map->m_flags |= EXT4_MAP_NEW;
	if (allocated > map->m_len) {
		unmap_underlying_metadata_blocks(inode->i_sb->s_bdev,
					newblock + map->m_len,
					allocated - map->m_len);
		allocated = map->m_len;
	}
	if (flags & EXT4_GET_BLOCKS_DELALLOC_RESERVE) {
		unsigned int reserved_clusters;
		reserved_clusters = get_reserved_cluster_alloc(inode,
				map->m_lblk, map->m_len);
		if (reserved_clusters)
			ext4_da_update_reserve_space(inode,
						     reserved_clusters,
						     0);
	}
map_out:
	map->m_flags |= EXT4_MAP_MAPPED;
	if ((flags & EXT4_GET_BLOCKS_KEEP_SIZE) == 0) {
		err = check_eofblocks_fl(handle, inode, map->m_lblk, path,
					 map->m_len);
		if (err < 0)
			goto out2;
	}
out1:
	if (allocated > map->m_len)
		allocated = map->m_len;
	ext4_ext_show_leaf(inode, path);
	map->m_pblk = newblock;
	map->m_len = allocated;
out2:
	if (path) {
		ext4_ext_drop_refs(path);
		kfree(path);
	}
	return err ? err : allocated;
}