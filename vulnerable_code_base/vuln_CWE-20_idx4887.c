int key_update(key_ref_t key_ref, const void *payload, size_t plen)
{
	struct key_preparsed_payload prep;
	struct key *key = key_ref_to_ptr(key_ref);
	int ret;
	key_check(key);
	ret = key_permission(key_ref, KEY_NEED_WRITE);
	if (ret < 0)
		return ret;
	if (!key->type->update)
		return -EOPNOTSUPP;
	memset(&prep, 0, sizeof(prep));
	prep.data = payload;
	prep.datalen = plen;
	prep.quotalen = key->type->def_datalen;
	prep.expiry = TIME_T_MAX;
	if (key->type->preparse) {
		ret = key->type->preparse(&prep);
		if (ret < 0)
			goto error;
	}
	down_write(&key->sem);
	ret = key->type->update(key, &prep);
	if (ret == 0)
		clear_bit(KEY_FLAG_NEGATIVE, &key->flags);
	up_write(&key->sem);
error:
	if (key->type->preparse)
		key->type->free_preparse(&prep);
	return ret;
}