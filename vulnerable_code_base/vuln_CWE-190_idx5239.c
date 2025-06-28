int bn_size_bin(const bn_t a) {
	dig_t d;
	int digits;
	digits = (a->used - 1) * (RLC_DIG / 8);
	d = a->dp[a->used - 1];
	while (d != 0) {
		d = d >> 8;
		digits++;
	}
	return digits;
}