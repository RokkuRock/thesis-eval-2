void check_system_chunk(struct btrfs_trans_handle *trans, u64 type)
{
	struct btrfs_transaction *cur_trans = trans->transaction;
	struct btrfs_fs_info *fs_info = trans->fs_info;
	struct btrfs_space_info *info;
	u64 left;
	u64 thresh;
	int ret = 0;
	u64 num_devs;
	lockdep_assert_held(&fs_info->chunk_mutex);
	info = btrfs_find_space_info(fs_info, BTRFS_BLOCK_GROUP_SYSTEM);
again:
	spin_lock(&info->lock);
	left = info->total_bytes - btrfs_space_info_used(info, true);
	spin_unlock(&info->lock);
	num_devs = get_profile_num_devs(fs_info, type);
	thresh = btrfs_calc_metadata_size(fs_info, num_devs) +
		btrfs_calc_insert_metadata_size(fs_info, 1);
	if (left < thresh && btrfs_test_opt(fs_info, ENOSPC_DEBUG)) {
		btrfs_info(fs_info, "left=%llu, need=%llu, flags=%llu",
			   left, thresh, type);
		btrfs_dump_space_info(fs_info, info, 0, 0);
	}
	if (left < thresh) {
		u64 flags = btrfs_system_alloc_profile(fs_info);
		u64 reserved = atomic64_read(&cur_trans->chunk_bytes_reserved);
		if (reserved > trans->chunk_bytes_reserved) {
			const u64 min_needed = reserved - thresh;
			mutex_unlock(&fs_info->chunk_mutex);
			wait_event(cur_trans->chunk_reserve_wait,
			   atomic64_read(&cur_trans->chunk_bytes_reserved) <=
			   min_needed);
			mutex_lock(&fs_info->chunk_mutex);
			goto again;
		}
		ret = btrfs_alloc_chunk(trans, flags);
	}
	if (!ret) {
		ret = btrfs_block_rsv_add(fs_info->chunk_root,
					  &fs_info->chunk_block_rsv,
					  thresh, BTRFS_RESERVE_NO_FLUSH);
		if (!ret) {
			atomic64_add(thresh, &cur_trans->chunk_bytes_reserved);
			trans->chunk_bytes_reserved += thresh;
		}
	}
}