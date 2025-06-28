long keyctl_read_key(key_serial_t keyid, char __user *buffer, size_t buflen)
{
	struct key *key;
	key_ref_t key_ref;
	long ret;
	key_ref = lookup_user_key(keyid, 0, 0);
	if (IS_ERR(key_ref)) {
		ret = -ENOKEY;
		goto error;
	}
	key = key_ref_to_ptr(key_ref);
	ret = key_permission(key_ref, KEY_NEED_READ);
	if (ret == 0)
		goto can_read_key;
	if (ret != -EACCES)
		goto error;
	if (!is_key_possessed(key_ref)) {
		ret = -EACCES;
		goto error2;
	}
can_read_key:
	ret = key_validate(key);
	if (ret == 0) {
		ret = -EOPNOTSUPP;
		if (key->type->read) {
			down_read(&key->sem);
			ret = key->type->read(key, buffer, buflen);
			up_read(&key->sem);
		}
	}
error2:
	key_put(key);
error:
	return ret;
}