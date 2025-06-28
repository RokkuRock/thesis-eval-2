static char get_bits(const bn_t a, int from, int to) {
	int f, t;
	dig_t mf, mt;
	RLC_RIP(from, f, from);
	RLC_RIP(to, t, to);
	if (f == t) {
		mf = RLC_MASK(from);
		if (to + 1 >= RLC_DIG) {
			mt = RLC_DMASK;
		} else {
			mt = RLC_MASK(to + 1);
		}
		mf = mf ^ mt;
		return ((a->dp[f] & (mf)) >> from);
	} else {
		mf = RLC_MASK(RLC_DIG - from) << from;
		mt = RLC_MASK(to + 1);
		return ((a->dp[f] & mf) >> from) |
				((a->dp[t] & mt) << (RLC_DIG - from));
	}
}