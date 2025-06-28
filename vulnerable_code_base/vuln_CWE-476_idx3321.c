static void mem_cgroup_usage_unregister_event(struct cgroup *cgrp,
	struct cftype *cft, struct eventfd_ctx *eventfd)
{
	struct mem_cgroup *memcg = mem_cgroup_from_cont(cgrp);
	struct mem_cgroup_thresholds *thresholds;
	struct mem_cgroup_threshold_ary *new;
	int type = MEMFILE_TYPE(cft->private);
	u64 usage;
	int i, j, size;
	mutex_lock(&memcg->thresholds_lock);
	if (type == _MEM)
		thresholds = &memcg->thresholds;
	else if (type == _MEMSWAP)
		thresholds = &memcg->memsw_thresholds;
	else
		BUG();
	BUG_ON(!thresholds);
	usage = mem_cgroup_usage(memcg, type == _MEMSWAP);
	__mem_cgroup_threshold(memcg, type == _MEMSWAP);
	size = 0;
	for (i = 0; i < thresholds->primary->size; i++) {
		if (thresholds->primary->entries[i].eventfd != eventfd)
			size++;
	}
	new = thresholds->spare;
	if (!size) {
		kfree(new);
		new = NULL;
		goto swap_buffers;
	}
	new->size = size;
	new->current_threshold = -1;
	for (i = 0, j = 0; i < thresholds->primary->size; i++) {
		if (thresholds->primary->entries[i].eventfd == eventfd)
			continue;
		new->entries[j] = thresholds->primary->entries[i];
		if (new->entries[j].threshold < usage) {
			++new->current_threshold;
		}
		j++;
	}
swap_buffers:
	thresholds->spare = thresholds->primary;
	rcu_assign_pointer(thresholds->primary, new);
	synchronize_rcu();
	mutex_unlock(&memcg->thresholds_lock);
}