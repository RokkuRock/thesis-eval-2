static ssize_t o2nm_node_local_store(struct config_item *item, const char *page,
				     size_t count)
{
	struct o2nm_node *node = to_o2nm_node(item);
	struct o2nm_cluster *cluster = to_o2nm_cluster_from_node(node);
	unsigned long tmp;
	char *p = (char *)page;
	ssize_t ret;
	tmp = simple_strtoul(p, &p, 0);
	if (!p || (*p && (*p != '\n')))
		return -EINVAL;
	tmp = !!tmp;  
	if (!test_bit(O2NM_NODE_ATTR_ADDRESS, &node->nd_set_attributes) ||
	    !test_bit(O2NM_NODE_ATTR_NUM, &node->nd_set_attributes) ||
	    !test_bit(O2NM_NODE_ATTR_PORT, &node->nd_set_attributes))
		return -EINVAL;  
	if (tmp && tmp == cluster->cl_has_local &&
	    cluster->cl_local_node != node->nd_num)
		return -EBUSY;
	if (tmp && !cluster->cl_has_local) {
		ret = o2net_start_listening(node);
		if (ret)
			return ret;
	}
	if (!tmp && cluster->cl_has_local &&
	    cluster->cl_local_node == node->nd_num) {
		o2net_stop_listening(node);
		cluster->cl_local_node = O2NM_INVALID_NODE_NUM;
	}
	node->nd_local = tmp;
	if (node->nd_local) {
		cluster->cl_has_local = tmp;
		cluster->cl_local_node = node->nd_num;
	}
	return count;
}