void bn_read_bin(bn_t a, const uint8_t *bin, int len) {
	int i, j;
	dig_t d = (RLC_DIG / 8);
	int digs = (len % d == 0 ? len / d : len / d + 1);
	bn_grow(a, digs);
	bn_zero(a);
	a->used = digs;
	for (i = 0; i < digs - 1; i++) {
		d = 0;
		for (j = (RLC_DIG / 8) - 1; j >= 0; j--) {
			d = d << 8;
			d |= bin[len - 1 - (i * (RLC_DIG / 8) + j)];
		}
		a->dp[i] = d;
	}
	d = 0;
	for (j = (RLC_DIG / 8) - 1; j >= 0; j--) {
		if ((int)(i * (RLC_DIG / 8) + j) < len) {
			d = d << 8;
			d |= bin[len - 1 - (i * (RLC_DIG / 8) + j)];
		}
	}
	a->dp[i] = d;
	a->sign = RLC_POS;
	bn_trim(a);
}