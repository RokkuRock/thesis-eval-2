static void show_object(struct object *obj,
			struct strbuf *path, const char *last,
			void *data)
{
	char *name = path_name(path, last);
	add_preferred_base_object(name);
	add_object_entry(obj->oid.hash, obj->type, name, 0);
	obj->flags |= OBJECT_ADDED;
	free((char *)name);
}