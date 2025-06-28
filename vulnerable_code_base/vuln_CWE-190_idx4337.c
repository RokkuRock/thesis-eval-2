int bn_get_bit(const bn_t a, int bit) {
	int d;
	if (bit < 0) {
		RLC_THROW(ERR_NO_VALID);
		return 0;
	}
	if (bit > bn_bits(a)) {
		return 0;
	}
	RLC_RIP(bit, d, bit);
	if (d >= a->used) {
		return 0;
	} else {
		return (a->dp[d] >> bit) & (dig_t)1;
	}
}