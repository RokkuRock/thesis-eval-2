static int oidc_cache_crypto_decrypt(request_rec *r, const char *cache_value,
		unsigned char *key, unsigned char **plaintext) {
	int len = -1;
	char *encoded_tag = strstr(cache_value, ".");
	if (encoded_tag == NULL) {
		oidc_error(r,
				"corrupted cache value: no tag separator found in encrypted value");
		return FALSE;
	}
	cache_value = apr_pstrmemdup(r->pool, cache_value,
			strlen(cache_value) - strlen(encoded_tag));
	encoded_tag++;
	char *d_bytes = NULL;
	int d_len = oidc_base64url_decode(r->pool, &d_bytes, cache_value);
	char *t_bytes = NULL;
	int t_len = oidc_base64url_decode(r->pool, &t_bytes, encoded_tag);
	if ((d_len > 0) && (t_len > 0)) {
		*plaintext = apr_pcalloc(r->pool,
				(d_len + EVP_CIPHER_block_size(OIDC_CACHE_CIPHER) - 1));
		len = oidc_cache_crypto_decrypt_impl(r, (unsigned char *) d_bytes,
				d_len, OIDC_CACHE_CRYPTO_GCM_AAD,
				sizeof(OIDC_CACHE_CRYPTO_GCM_AAD), (unsigned char *) t_bytes,
				t_len, key, OIDC_CACHE_CRYPTO_GCM_IV,
				sizeof(OIDC_CACHE_CRYPTO_GCM_IV), *plaintext);
		if (len > -1) {
			(*plaintext)[len] = '\0';
		} else {
			*plaintext = NULL;
		}
	}
	return len;
}