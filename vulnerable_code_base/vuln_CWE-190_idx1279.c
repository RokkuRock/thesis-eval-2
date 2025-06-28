void bn_set_2b(bn_t a, int b) {
	int i, d;
	if (b < 0) {
		bn_zero(a);
	} else {
		RLC_RIP(b, d, b);
		bn_grow(a, d + 1);
		for (i = 0; i < d; i++) {
			a->dp[i] = 0;
		}
		a->used = d + 1;
		a->dp[d] = ((dig_t)1 << b);
		a->sign = RLC_POS;
	}
}