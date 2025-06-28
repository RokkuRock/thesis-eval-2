static bool meta_set(RAnal *a, RAnalMetaType type, int subtype, ut64 from, ut64 to, const char *str) {
	if (to < from) {
		return false;
	}
	RSpace *space = r_spaces_current (&a->meta_spaces);
	RIntervalNode *node = find_node_at (a, type, space, from);
	RAnalMetaItem *item = node ? node->data : R_NEW0 (RAnalMetaItem);
	if (!item) {
		return false;
	}
	item->type = type;
	item->subtype = subtype;
	item->space = space;
	free (item->str);
	item->str = str ? strdup (str) : NULL;
	if (str && !item->str) {
		if (!node) {  
			free (item);
		}
		return false;
	}
	R_DIRTY (a);
	if (!node) {
		r_interval_tree_insert (&a->meta, from, to, item);
	} else if (node->end != to) {
		r_interval_tree_resize (&a->meta, node, from, to);
	}
	return true;
}