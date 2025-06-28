static int mark_source_chains(const struct xt_table_info *newinfo,
			      unsigned int valid_hooks, void *entry0)
{
	unsigned int hook;
	for (hook = 0; hook < NF_ARP_NUMHOOKS; hook++) {
		unsigned int pos = newinfo->hook_entry[hook];
		struct arpt_entry *e
			= (struct arpt_entry *)(entry0 + pos);
		if (!(valid_hooks & (1 << hook)))
			continue;
		e->counters.pcnt = pos;
		for (;;) {
			const struct xt_standard_target *t
				= (void *)arpt_get_target_c(e);
			int visited = e->comefrom & (1 << hook);
			if (e->comefrom & (1 << NF_ARP_NUMHOOKS)) {
				pr_notice("arptables: loop hook %u pos %u %08X.\n",
				       hook, pos, e->comefrom);
				return 0;
			}
			e->comefrom
				|= ((1 << hook) | (1 << NF_ARP_NUMHOOKS));
			if ((e->target_offset == sizeof(struct arpt_entry) &&
			     (strcmp(t->target.u.user.name,
				     XT_STANDARD_TARGET) == 0) &&
			     t->verdict < 0 && unconditional(&e->arp)) ||
			    visited) {
				unsigned int oldpos, size;
				if ((strcmp(t->target.u.user.name,
					    XT_STANDARD_TARGET) == 0) &&
				    t->verdict < -NF_MAX_VERDICT - 1) {
					duprintf("mark_source_chains: bad "
						"negative verdict (%i)\n",
								t->verdict);
					return 0;
				}
				do {
					e->comefrom ^= (1<<NF_ARP_NUMHOOKS);
					oldpos = pos;
					pos = e->counters.pcnt;
					e->counters.pcnt = 0;
					if (pos == oldpos)
						goto next;
					e = (struct arpt_entry *)
						(entry0 + pos);
				} while (oldpos == pos + e->next_offset);
				size = e->next_offset;
				e = (struct arpt_entry *)
					(entry0 + pos + size);
				e->counters.pcnt = pos;
				pos += size;
			} else {
				int newpos = t->verdict;
				if (strcmp(t->target.u.user.name,
					   XT_STANDARD_TARGET) == 0 &&
				    newpos >= 0) {
					if (newpos > newinfo->size -
						sizeof(struct arpt_entry)) {
						duprintf("mark_source_chains: "
							"bad verdict (%i)\n",
								newpos);
						return 0;
					}
					duprintf("Jump rule %u -> %u\n",
						 pos, newpos);
				} else {
					newpos = pos + e->next_offset;
				}
				e = (struct arpt_entry *)
					(entry0 + newpos);
				e->counters.pcnt = pos;
				pos = newpos;
			}
		}
next:
		duprintf("Finished chain %u\n", hook);
	}
	return 1;
}