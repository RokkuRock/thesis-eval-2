static struct o2nm_cluster *to_o2nm_cluster_from_node(struct o2nm_node *node)
{
	return to_o2nm_cluster(node->nd_item.ci_parent->ci_parent);
}