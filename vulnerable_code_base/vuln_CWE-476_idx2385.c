static int __init memory_tier_init(void)
{
	int ret, node;
	struct memory_tier *memtier;
	ret = subsys_virtual_register(&memory_tier_subsys, NULL);
	if (ret)
		panic("%s() failed to register memory tier subsystem\n", __func__);
#ifdef CONFIG_MIGRATION
	node_demotion = kcalloc(nr_node_ids, sizeof(struct demotion_nodes),
				GFP_KERNEL);
	WARN_ON(!node_demotion);
#endif
	mutex_lock(&memory_tier_lock);
	default_dram_type = alloc_memory_type(MEMTIER_ADISTANCE_DRAM);
	if (!default_dram_type)
		panic("%s() failed to allocate default DRAM tier\n", __func__);
	for_each_node_state(node, N_MEMORY) {
		memtier = set_node_memory_tier(node);
		if (IS_ERR(memtier))
			break;
	}
	establish_demotion_targets();
	mutex_unlock(&memory_tier_lock);
	hotplug_memory_notifier(memtier_hotplug_callback, MEMTIER_HOTPLUG_PRI);
	return 0;
}