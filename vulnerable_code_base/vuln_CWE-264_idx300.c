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
	struct uid_gid_extent *extent = NULL;
	unsigned long page = 0;
	char *kbuf, *pos, *next_line;
	ssize_t ret = -EINVAL;
	mutex_lock(&id_map_mutex);
	ret = -EPERM;
	if (map->nr_extents != 0)
		goto out;
	if (cap_valid(cap_setid) && !ns_capable(ns, cap_setid))
		goto out;
	ret = -ENOMEM;
	page = __get_free_page(GFP_TEMPORARY);
	kbuf = (char *) page;
	if (!page)
		goto out;
	ret = -EINVAL;
	if ((*ppos != 0) || (count >= PAGE_SIZE))
		goto out;
	ret = -EFAULT;
	if (copy_from_user(kbuf, buf, count))
		goto out;
	kbuf[count] = '\0';
	ret = -EINVAL;
	pos = kbuf;
	new_map.nr_extents = 0;
	for (;pos; pos = next_line) {
		extent = &new_map.extent[new_map.nr_extents];
		next_line = strchr(pos, '\n');
		if (next_line) {
			*next_line = '\0';
			next_line++;
			if (*next_line == '\0')
				next_line = NULL;
		}
		pos = skip_spaces(pos);
		extent->first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		extent->lower_first = simple_strtoul(pos, &pos, 10);
		if (!isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		extent->count = simple_strtoul(pos, &pos, 10);
		if (*pos && !isspace(*pos))
			goto out;
		pos = skip_spaces(pos);
		if (*pos != '\0')
			goto out;
		if ((extent->first == (u32) -1) ||
		    (extent->lower_first == (u32) -1 ))
			goto out;
		if ((extent->first + extent->count) <= extent->first)
			goto out;
		if ((extent->lower_first + extent->count) <= extent->lower_first)
			goto out;
		if (mappings_overlap(&new_map, extent))
			goto out;
		new_map.nr_extents++;
		if ((new_map.nr_extents == UID_GID_MAP_MAX_EXTENTS) &&
		    (next_line != NULL))
			goto out;
	}
	if (new_map.nr_extents == 0)
		goto out;
	ret = -EPERM;
	if (!new_idmap_permitted(ns, cap_setid, &new_map))
		goto out;
	for (idx = 0; idx < new_map.nr_extents; idx++) {
		u32 lower_first;
		extent = &new_map.extent[idx];
		lower_first = map_id_range_down(parent_map,
						extent->lower_first,
						extent->count);
		if (lower_first == (u32) -1)
			goto out;
		extent->lower_first = lower_first;
	}
	memcpy(map->extent, new_map.extent,
		new_map.nr_extents*sizeof(new_map.extent[0]));
	smp_wmb();
	map->nr_extents = new_map.nr_extents;
	*ppos = count;
	ret = count;
out:
	mutex_unlock(&id_map_mutex);
	if (page)
		free_page(page);
	return ret;
}