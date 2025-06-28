void bn_write_raw(dig_t *raw, int len, const bn_t a) {
	int i, size;
	size = a->used;
	if (len < size) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	for (i = 0; i < size; i++) {
		raw[i] = a->dp[i];
	}
	for (; i < len; i++) {
		raw[i] = 0;
	}
}