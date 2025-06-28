static int asymmetric_key_match(const struct key *key,
				const struct key_match_data *match_data)
{
	const struct asymmetric_key_subtype *subtype = asymmetric_key_subtype(key);
	const char *description = match_data->raw_data;
	const char *spec = description;
	const char *id;
	ptrdiff_t speclen;
	if (!subtype || !spec || !*spec)
		return 0;
	if (key->description && strcmp(key->description, description) == 0)
		return 1;
	id = strchr(spec, ':');
	if (!id)
		return 0;
	speclen = id - spec;
	id++;
	if (speclen == 2 && memcmp(spec, "id", 2) == 0)
		return asymmetric_keyid_match(asymmetric_key_id(key), id);
	if (speclen == subtype->name_len &&
	    memcmp(spec, subtype->name, speclen) == 0)
		return 1;
	return 0;
}