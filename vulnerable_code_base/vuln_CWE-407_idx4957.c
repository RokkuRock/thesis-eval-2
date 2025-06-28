add_link_ref(
	struct link_ref **references,
	const uint8_t *name, size_t name_size)
{
	struct link_ref *ref = calloc(1, sizeof(struct link_ref));
	if (!ref)
		return NULL;
	ref->id = hash_link_ref(name, name_size);
	ref->next = references[ref->id % REF_TABLE_SIZE];
	references[ref->id % REF_TABLE_SIZE] = ref;
	return ref;
}