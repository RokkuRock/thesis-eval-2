int user_update(struct key *key, struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload, *zap;
	size_t datalen = prep->datalen;
	int ret;
	ret = -EINVAL;
	if (datalen <= 0 || datalen > 32767 || !prep->data)
		goto error;
	ret = -ENOMEM;
	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		goto error;
	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);
	zap = upayload;
	ret = key_payload_reserve(key, datalen);
	if (ret == 0) {
		zap = key->payload.data[0];
		rcu_assign_keypointer(key, upayload);
		key->expiry = 0;
	}
	if (zap)
		kfree_rcu(zap, rcu);
error:
	return ret;
}