int jpc_ppxstab_insert(jpc_ppxstab_t *tab, jpc_ppxstabent_t *ent)
{
	int inspt;
	int i;
	for (i = 0; i < tab->numents; ++i) {
		if (tab->ents[i]->ind > ent->ind) {
			break;
		}
	}
	inspt = i;
	if (tab->numents >= tab->maxents) {
		if (jpc_ppxstab_grow(tab, tab->maxents + 128)) {
			return -1;
		}
	}
	for (i = tab->numents; i > inspt; --i) {
		tab->ents[i] = tab->ents[i - 1];
	}
	tab->ents[i] = ent;
	++tab->numents;
	return 0;
}