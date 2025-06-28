int install_user_keyrings(void)
{
	struct user_struct *user;
	const struct cred *cred;
	struct key *uid_keyring, *session_keyring;
	key_perm_t user_keyring_perm;
	char buf[20];
	int ret;
	uid_t uid;
	user_keyring_perm = (KEY_POS_ALL & ~KEY_POS_SETATTR) | KEY_USR_ALL;
	cred = current_cred();
	user = cred->user;
	uid = from_kuid(cred->user_ns, user->uid);
	kenter("%p{%u}", user, uid);
	if (user->uid_keyring) {
		kleave(" = 0 [exist]");
		return 0;
	}
	mutex_lock(&key_user_keyring_mutex);
	ret = 0;
	if (!user->uid_keyring) {
		sprintf(buf, "_uid.%u", uid);
		uid_keyring = find_keyring_by_name(buf, true);
		if (IS_ERR(uid_keyring)) {
			uid_keyring = keyring_alloc(buf, user->uid, INVALID_GID,
						    cred, user_keyring_perm,
						    KEY_ALLOC_IN_QUOTA, NULL);
			if (IS_ERR(uid_keyring)) {
				ret = PTR_ERR(uid_keyring);
				goto error;
			}
		}
		sprintf(buf, "_uid_ses.%u", uid);
		session_keyring = find_keyring_by_name(buf, true);
		if (IS_ERR(session_keyring)) {
			session_keyring =
				keyring_alloc(buf, user->uid, INVALID_GID,
					      cred, user_keyring_perm,
					      KEY_ALLOC_IN_QUOTA, NULL);
			if (IS_ERR(session_keyring)) {
				ret = PTR_ERR(session_keyring);
				goto error_release;
			}
			ret = key_link(session_keyring, uid_keyring);
			if (ret < 0)
				goto error_release_both;
		}
		user->uid_keyring = uid_keyring;
		user->session_keyring = session_keyring;
	}
	mutex_unlock(&key_user_keyring_mutex);
	kleave(" = 0");
	return 0;
error_release_both:
	key_put(session_keyring);
error_release:
	key_put(uid_keyring);
error:
	mutex_unlock(&key_user_keyring_mutex);
	kleave(" = %d", ret);
	return ret;
}