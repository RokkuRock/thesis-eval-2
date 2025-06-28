int btrfs_search_slot(struct btrfs_trans_handle *trans, struct btrfs_root
		      *root, struct btrfs_key *key, struct btrfs_path *p, int
		      ins_len, int cow)
{
	struct extent_buffer *b;
	int slot;
	int ret;
	int err;
	int level;
	int lowest_unlock = 1;
	int root_lock;
	int write_lock_level = 0;
	u8 lowest_level = 0;
	int min_write_lock_level;
	int prev_cmp;
	lowest_level = p->lowest_level;
	WARN_ON(lowest_level && ins_len > 0);
	WARN_ON(p->nodes[0] != NULL);
	BUG_ON(!cow && ins_len);
	if (ins_len < 0) {
		lowest_unlock = 2;
		write_lock_level = 2;
	} else if (ins_len > 0) {
		write_lock_level = 1;
	}
	if (!cow)
		write_lock_level = -1;
	if (cow && (p->keep_locks || p->lowest_level))
		write_lock_level = BTRFS_MAX_LEVEL;
	min_write_lock_level = write_lock_level;
again:
	prev_cmp = -1;
	root_lock = BTRFS_READ_LOCK;
	level = 0;
	if (p->search_commit_root) {
		if (p->need_commit_sem)
			down_read(&root->fs_info->commit_root_sem);
		b = root->commit_root;
		extent_buffer_get(b);
		level = btrfs_header_level(b);
		if (p->need_commit_sem)
			up_read(&root->fs_info->commit_root_sem);
		if (!p->skip_locking)
			btrfs_tree_read_lock(b);
	} else {
		if (p->skip_locking) {
			b = btrfs_root_node(root);
			level = btrfs_header_level(b);
		} else {
			b = btrfs_read_lock_root_node(root);
			level = btrfs_header_level(b);
			if (level <= write_lock_level) {
				btrfs_tree_read_unlock(b);
				free_extent_buffer(b);
				b = btrfs_lock_root_node(root);
				root_lock = BTRFS_WRITE_LOCK;
				level = btrfs_header_level(b);
			}
		}
	}
	p->nodes[level] = b;
	if (!p->skip_locking)
		p->locks[level] = root_lock;
	while (b) {
		level = btrfs_header_level(b);
		if (cow) {
			if (!should_cow_block(trans, root, b))
				goto cow_done;
			if (level > write_lock_level ||
			    (level + 1 > write_lock_level &&
			    level + 1 < BTRFS_MAX_LEVEL &&
			    p->nodes[level + 1])) {
				write_lock_level = level + 1;
				btrfs_release_path(p);
				goto again;
			}
			btrfs_set_path_blocking(p);
			err = btrfs_cow_block(trans, root, b,
					      p->nodes[level + 1],
					      p->slots[level + 1], &b);
			if (err) {
				ret = err;
				goto done;
			}
		}
cow_done:
		p->nodes[level] = b;
		btrfs_clear_path_blocking(p, NULL, 0);
		if (!ins_len && !p->keep_locks) {
			int u = level + 1;
			if (u < BTRFS_MAX_LEVEL && p->locks[u]) {
				btrfs_tree_unlock_rw(p->nodes[u], p->locks[u]);
				p->locks[u] = 0;
			}
		}
		ret = key_search(b, key, level, &prev_cmp, &slot);
		if (level != 0) {
			int dec = 0;
			if (ret && slot > 0) {
				dec = 1;
				slot -= 1;
			}
			p->slots[level] = slot;
			err = setup_nodes_for_search(trans, root, p, b, level,
					     ins_len, &write_lock_level);
			if (err == -EAGAIN)
				goto again;
			if (err) {
				ret = err;
				goto done;
			}
			b = p->nodes[level];
			slot = p->slots[level];
			if (slot == 0 && ins_len &&
			    write_lock_level < level + 1) {
				write_lock_level = level + 1;
				btrfs_release_path(p);
				goto again;
			}
			unlock_up(p, level, lowest_unlock,
				  min_write_lock_level, &write_lock_level);
			if (level == lowest_level) {
				if (dec)
					p->slots[level]++;
				goto done;
			}
			err = read_block_for_search(trans, root, p,
						    &b, level, slot, key, 0);
			if (err == -EAGAIN)
				goto again;
			if (err) {
				ret = err;
				goto done;
			}
			if (!p->skip_locking) {
				level = btrfs_header_level(b);
				if (level <= write_lock_level) {
					err = btrfs_try_tree_write_lock(b);
					if (!err) {
						btrfs_set_path_blocking(p);
						btrfs_tree_lock(b);
						btrfs_clear_path_blocking(p, b,
								  BTRFS_WRITE_LOCK);
					}
					p->locks[level] = BTRFS_WRITE_LOCK;
				} else {
					err = btrfs_try_tree_read_lock(b);
					if (!err) {
						btrfs_set_path_blocking(p);
						btrfs_tree_read_lock(b);
						btrfs_clear_path_blocking(p, b,
								  BTRFS_READ_LOCK);
					}
					p->locks[level] = BTRFS_READ_LOCK;
				}
				p->nodes[level] = b;
			}
		} else {
			p->slots[level] = slot;
			if (ins_len > 0 &&
			    btrfs_leaf_free_space(root, b) < ins_len) {
				if (write_lock_level < 1) {
					write_lock_level = 1;
					btrfs_release_path(p);
					goto again;
				}
				btrfs_set_path_blocking(p);
				err = split_leaf(trans, root, key,
						 p, ins_len, ret == 0);
				btrfs_clear_path_blocking(p, NULL, 0);
				BUG_ON(err > 0);
				if (err) {
					ret = err;
					goto done;
				}
			}
			if (!p->search_for_split)
				unlock_up(p, level, lowest_unlock,
					  min_write_lock_level, &write_lock_level);
			goto done;
		}
	}
	ret = 1;
done:
	if (!p->leave_spinning)
		btrfs_set_path_blocking(p);
	if (ret < 0)
		btrfs_release_path(p);
	return ret;
}