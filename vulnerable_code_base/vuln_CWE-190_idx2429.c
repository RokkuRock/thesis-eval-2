int bn_bits(const bn_t a) {
	int bits;
	if (bn_is_zero(a)) {
		return 0;
	}
	bits = (a->used - 1) * RLC_DIG;
	return bits + util_bits_dig(a->dp[a->used - 1]);
}