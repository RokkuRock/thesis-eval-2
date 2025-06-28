void bn_gen_prime_safep(bn_t a, int bits) {
	while (1) {
		do {
			bn_rand(a, RLC_POS, bits);
		} while (bn_bits(a) != bits);
		bn_sub_dig(a, a, 1);
		bn_rsh(a, a, 1);
		if (bn_is_prime(a)) {
			bn_lsh(a, a, 1);
			bn_add_dig(a, a, 1);
			if (bn_is_prime(a)) {
				return;
			}
		}
	}
}