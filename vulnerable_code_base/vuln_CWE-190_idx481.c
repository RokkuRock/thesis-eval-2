void bn_set_bit(bn_t a, int bit, int value) {
	int d;
	if (bit < 0) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	RLC_RIP(bit, d, bit);
	bn_grow(a, d);
	if (value == 1) {
		a->dp[d] |= ((dig_t)1 << bit);
		if ((d + 1) > a->used) {
			a->used = d + 1;
		}
	} else {
		a->dp[d] &= ~((dig_t)1 << bit);
		bn_trim(a);
	}
}