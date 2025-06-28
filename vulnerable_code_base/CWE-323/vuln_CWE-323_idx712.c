static unsigned char *oidc_cache_hash_passphrase(request_rec *r,
		const char *passphrase) {
	unsigned char *key = NULL;
	unsigned int key_len = 0;
	oidc_jose_error_t err;
	if (oidc_jose_hash_bytes(r->pool, OIDC_JOSE_ALG_SHA256,
			(const unsigned char *) passphrase, strlen(passphrase), &key,
			&key_len, &err) == FALSE) {
		oidc_error(r, "oidc_jose_hash_bytes returned an error: %s", err.text);
		return NULL;
	}
	return key;
}