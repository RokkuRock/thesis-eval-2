void md_kdf(uint8_t *key, int key_len, const uint8_t *in,
		int in_len) {
	uint32_t i, j, d;
	uint8_t* buffer = RLC_ALLOCA(uint8_t, in_len + sizeof(uint32_t));
	uint8_t* t = RLC_ALLOCA(uint8_t, key_len + RLC_MD_LEN);
	if (buffer == NULL || t == NULL) {
		RLC_FREE(buffer);
		RLC_FREE(t);
		RLC_THROW(ERR_NO_MEMORY);
		return;
	}
	d = RLC_CEIL(key_len, RLC_MD_LEN);
	memcpy(buffer, in, in_len);
	for (i = 1; i <= d; i++) {
		j = util_conv_big(i);
		memcpy(buffer + in_len, &j, sizeof(uint32_t));
		md_map(t + (i - 1) * RLC_MD_LEN, buffer, in_len + sizeof(uint32_t));
	}
	memcpy(key, t, key_len);
	RLC_FREE(buffer);
	RLC_FREE(t);
}