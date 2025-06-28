find_link_ref(struct link_ref **references, uint8_t *name, size_t length)
{
	unsigned int hash = hash_link_ref(name, length);
	struct link_ref *ref = NULL;
	ref = references[hash % REF_TABLE_SIZE];
	while (ref != NULL) {
		if (ref->id == hash)
			return ref;
		ref = ref->next;
	}
	return NULL;
}