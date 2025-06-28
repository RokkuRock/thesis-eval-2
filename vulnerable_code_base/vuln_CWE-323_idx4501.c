static int oidc_cache_crypto_decrypt_impl(request_rec *r,
		unsigned char *ciphertext, int ciphertext_len, const unsigned char *aad,
		int aad_len, const unsigned char *tag, int tag_len, unsigned char *key,
		const unsigned char *iv, int iv_len, unsigned char *plaintext) {
	EVP_CIPHER_CTX *ctx;
	int len;
	int plaintext_len;
	int ret;
	if (!(ctx = EVP_CIPHER_CTX_new())) {
		oidc_cache_crypto_openssl_error(r, "EVP_CIPHER_CTX_new");
		return -1;
	}
	if (!EVP_DecryptInit_ex(ctx, OIDC_CACHE_CIPHER, NULL, NULL, NULL)) {
		oidc_cache_crypto_openssl_error(r, "EVP_DecryptInit_ex");
		return -1;
	}
	if (!EVP_CIPHER_CTX_ctrl(ctx, OIDC_CACHE_CRYPTO_SET_IVLEN, iv_len, NULL)) {
		oidc_cache_crypto_openssl_error(r, "EVP_CIPHER_CTX_ctrl");
		return -1;
	}
	if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
		oidc_cache_crypto_openssl_error(r, "EVP_DecryptInit_ex");
		return -1;
	}
	if (!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len)) {
		oidc_cache_crypto_openssl_error(r, "EVP_DecryptUpdate aad: aad_len=%d",
				aad_len);
		return -1;
	}
	if (!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
		oidc_cache_crypto_openssl_error(r, "EVP_DecryptUpdate ciphertext");
		return -1;
	}
	plaintext_len = len;
	if (!EVP_CIPHER_CTX_ctrl(ctx, OIDC_CACHE_CRYPTO_SET_TAG, tag_len,
			(void *) tag)) {
		oidc_cache_crypto_openssl_error(r, "EVP_CIPHER_CTX_ctrl");
		return -1;
	}
	ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
	EVP_CIPHER_CTX_free(ctx);
	if (ret > 0) {
		plaintext_len += len;
		return plaintext_len;
	} else {
		oidc_cache_crypto_openssl_error(r, "EVP_DecryptFinal_ex");
		return -1;
	}
}