void bn_gen_prime_basic(bn_t a, int bits) {
	while (1) {
		do {
			bn_rand(a, RLC_POS, bits);
		} while (bn_bits(a) != bits);
		if (bn_is_prime(a)) {
			return;
		}
	}
}