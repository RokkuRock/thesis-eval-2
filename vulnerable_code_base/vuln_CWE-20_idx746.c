static int __key_instantiate_and_link(struct key *key,
				      struct key_preparsed_payload *prep,
				      struct key *keyring,
				      struct key *authkey,
				      struct assoc_array_edit **_edit)
{
	int ret, awaken;
	key_check(key);
	key_check(keyring);
	awaken = 0;
	ret = -EBUSY;
	mutex_lock(&key_construction_mutex);
	if (!test_bit(KEY_FLAG_INSTANTIATED, &key->flags)) {
		ret = key->type->instantiate(key, prep);
		if (ret == 0) {
			atomic_inc(&key->user->nikeys);
			set_bit(KEY_FLAG_INSTANTIATED, &key->flags);
			if (test_and_clear_bit(KEY_FLAG_USER_CONSTRUCT, &key->flags))
				awaken = 1;
			if (keyring) {
				if (test_bit(KEY_FLAG_KEEP, &keyring->flags))
					set_bit(KEY_FLAG_KEEP, &key->flags);
				__key_link(key, _edit);
			}
			if (authkey)
				key_revoke(authkey);
			if (prep->expiry != TIME_T_MAX) {
				key->expiry = prep->expiry;
				key_schedule_gc(prep->expiry + key_gc_delay);
			}
		}
	}
	mutex_unlock(&key_construction_mutex);
	if (awaken)
		wake_up_bit(&key->flags, KEY_FLAG_USER_CONSTRUCT);
	return ret;
}