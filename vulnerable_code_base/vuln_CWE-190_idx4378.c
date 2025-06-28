void bn_trim(bn_t a) {
	if (a->used <= a->alloc) {
		while (a->used > 0 && a->dp[a->used - 1] == 0) {
			--(a->used);
		}
		if (a->used <= 0) {
			a->used = 1;
			a->dp[0] = 0;
			a->sign = RLC_POS;
		}
	}
}