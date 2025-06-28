void bn_rand(bn_t a, int sign, int bits) {
	int digits;
	RLC_RIP(bits, digits, bits);
	digits += (bits > 0 ? 1 : 0);
	bn_grow(a, digits);
	rand_bytes((uint8_t *)a->dp, digits * sizeof(dig_t));
	a->used = digits;
	a->sign = sign;
	if (bits > 0) {
		dig_t mask = ((dig_t)1 << (dig_t)bits) - 1;
		a->dp[a->used - 1] &= mask;
	}
	bn_trim(a);
}