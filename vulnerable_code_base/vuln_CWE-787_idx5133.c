int mpol_parse_str(char *str, struct mempolicy **mpol)
{
	struct mempolicy *new = NULL;
	unsigned short mode_flags;
	nodemask_t nodes;
	char *nodelist = strchr(str, ':');
	char *flags = strchr(str, '=');
	int err = 1, mode;
	if (flags)
		*flags++ = '\0';	 
	if (nodelist) {
		*nodelist++ = '\0';
		if (nodelist_parse(nodelist, nodes))
			goto out;
		if (!nodes_subset(nodes, node_states[N_MEMORY]))
			goto out;
	} else
		nodes_clear(nodes);
	mode = match_string(policy_modes, MPOL_MAX, str);
	if (mode < 0)
		goto out;
	switch (mode) {
	case MPOL_PREFERRED:
		if (nodelist) {
			char *rest = nodelist;
			while (isdigit(*rest))
				rest++;
			if (*rest)
				goto out;
		}
		break;
	case MPOL_INTERLEAVE:
		if (!nodelist)
			nodes = node_states[N_MEMORY];
		break;
	case MPOL_LOCAL:
		if (nodelist)
			goto out;
		mode = MPOL_PREFERRED;
		break;
	case MPOL_DEFAULT:
		if (!nodelist)
			err = 0;
		goto out;
	case MPOL_BIND:
		if (!nodelist)
			goto out;
	}
	mode_flags = 0;
	if (flags) {
		if (!strcmp(flags, "static"))
			mode_flags |= MPOL_F_STATIC_NODES;
		else if (!strcmp(flags, "relative"))
			mode_flags |= MPOL_F_RELATIVE_NODES;
		else
			goto out;
	}
	new = mpol_new(mode, mode_flags, &nodes);
	if (IS_ERR(new))
		goto out;
	if (mode != MPOL_PREFERRED)
		new->v.nodes = nodes;
	else if (nodelist)
		new->v.preferred_node = first_node(nodes);
	else
		new->flags |= MPOL_F_LOCAL;
	new->w.user_nodemask = nodes;
	err = 0;
out:
	if (nodelist)
		*--nodelist = ':';
	if (flags)
		*--flags = '=';
	if (!err)
		*mpol = new;
	return err;
}