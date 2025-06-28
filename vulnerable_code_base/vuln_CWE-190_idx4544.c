void bn_write_bin(uint8_t *bin, int len, const bn_t a) {
	int size, k;
	dig_t d;
	size = bn_size_bin(a);
	if (len < size) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	k = 0;
	for (int i = 0; i < a->used - 1; i++) {
		d = a->dp[i];
		for (int j = 0; j < (int)(RLC_DIG / 8); j++) {
			bin[len - 1 - k++] = d & 0xFF;
			d = d >> 8;
		}
	}
	d = a->dp[a->used - 1];
	while (d != 0) {
		bin[len - 1 - k++] = d & 0xFF;
		d = d >> 8;
	}
	while (k < len) {
		bin[len - 1 - k++] = 0;
	}
}