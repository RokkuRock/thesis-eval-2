static ssize_t map_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos,
			 int cap_setid,
			 struct uid_gid_map *map,
			 struct uid_gid_map *parent_map)
{
	struct seq_file *seq = file->private_data;
	struct user_namespace *ns = seq->private;
	struct uid_gid_map new_map;
	unsigned idx;
	struct uid_gid_extent extent;
	char *kbuf = NULL, *pos, *next_line;
	ssize_t ret;
	if ((*ppos != 0) || (count >= PAGE_SIZE))
		return -EINVAL;
	kbuf = memdup_user_nul(buf, count);
	if (IS_ERR(kbuf))
		return PTR_ERR(kbuf);
	mutex_lock(&userns_state_mutex);
	memset(&new_map, 0, sizeof(struct uid_gid_map));
	ret = -EPERM;
	if (map->nr_extents != 0)
		goto out;
	if (cap_valid(cap_setid) && !file_ns_capable(file, ns, CAP_SYS_ADMIN))
		goto out;
	ret = -EINVAL;
	pos = kbuf;
	for (; pos; pos = next_line) {
		next_line = strchr(pos, '\n');
		if (next_line) {
			*next_line = '\0';
			next_line++;
			if (*next_line == '\0')
				next_line = NULL;
		}
		pos = skip_spaces(pos);
		extent.first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		extent.lower_first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		extent.count = simple_strtoul(pos, &pos, 10);
		if (*pos && !isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		if (*pos != '\0')
			goto out;
		if ((extent.first == (u32) -1) ||
		    (extent.lower_first == (u32) -1))
			goto out;
		if ((extent.first + extent.count) <= extent.first)
			goto out;
		if ((extent.lower_first + extent.count) <=
		     extent.lower_first)
			goto out;
		if (mappings_overlap(&new_map, &extent))
			goto out;
		if ((new_map.nr_extents + 1) == UID_GID_MAP_MAX_EXTENTS &&
		    (next_line != NULL))
			goto out;
		ret = insert_extent(&new_map, &extent);
		if (ret < 0)
			goto out;
		ret = -EINVAL;
	}
	if (new_map.nr_extents == 0)
		goto out;
	ret = -EPERM;
	if (!new_idmap_permitted(file, ns, cap_setid, &new_map))
		goto out;
	ret = sort_idmaps(&new_map);
	if (ret < 0)
		goto out;
	ret = -EPERM;
	for (idx = 0; idx < new_map.nr_extents; idx++) {
		struct uid_gid_extent *e;
		u32 lower_first;
		if (new_map.nr_extents <= UID_GID_MAP_MAX_BASE_EXTENTS)
			e = &new_map.extent[idx];
		else
			e = &new_map.forward[idx];
		lower_first = map_id_range_down(parent_map,
						e->lower_first,
						e->count);
		if (lower_first == (u32) -1)
			goto out;
		e->lower_first = lower_first;
	}
	if (new_map.nr_extents <= UID_GID_MAP_MAX_BASE_EXTENTS) {
		memcpy(map->extent, new_map.extent,
		       new_map.nr_extents * sizeof(new_map.extent[0]));
	} else {
		map->forward = new_map.forward;
		map->reverse = new_map.reverse;
	}
	smp_wmb();
	map->nr_extents = new_map.nr_extents;
	*ppos = count;
	ret = count;
out:
	if (ret < 0 && new_map.nr_extents > UID_GID_MAP_MAX_BASE_EXTENTS) {
		kfree(new_map.forward);
		kfree(new_map.reverse);
		map->forward = NULL;
		map->reverse = NULL;
		map->nr_extents = 0;
	}
	mutex_unlock(&userns_state_mutex);
	kfree(kbuf);
	return ret;
}