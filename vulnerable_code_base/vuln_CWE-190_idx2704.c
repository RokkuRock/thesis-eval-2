void bn_gen_prime_stron(bn_t a, int bits) {
	dig_t i, j;
	int found, k;
	bn_t r, s, t;
	bn_null(r);
	bn_null(s);
	bn_null(t);
	RLC_TRY {
		bn_new(r);
		bn_new(s);
		bn_new(t);
		do {
			do {
				bn_rand(s, RLC_POS, bits / 2 - RLC_DIG / 2);
				bn_rand(t, RLC_POS, bits / 2 - RLC_DIG / 2);
			} while (!bn_is_prime(s) || !bn_is_prime(t));
			found = 1;
			bn_rand(a, RLC_POS, bits / 2 - bn_bits(t) - 1);
			i = a->dp[0];
			bn_dbl(t, t);
			do {
				bn_mul_dig(r, t, i);
				bn_add_dig(r, r, 1);
				i++;
				if (bn_bits(r) > bits / 2 - 1) {
					found = 0;
					break;
				}
			} while (!bn_is_prime(r));
			if (found == 0) {
				continue;
			}
			bn_sub_dig(t, r, 2);
#if BN_MOD != PMERS
			bn_mxp(t, s, t, r);
#else
			bn_exp(t, s, t, r);
#endif
			bn_mul(t, t, s);
			bn_dbl(t, t);
			bn_sub_dig(t, t, 1);
			k = bits - bn_bits(r);
			k -= bn_bits(s);
			bn_rand(a, RLC_POS, k);
			j = a->dp[0];
			do {
				bn_mul(a, r, s);
				bn_mul_dig(a, a, j);
				bn_dbl(a, a);
				bn_add(a, a, t);
				j++;
				if (bn_bits(a) > bits) {
					found = 0;
					break;
				}
			} while (!bn_is_prime(a));
		} while (found == 0 && bn_bits(a) != bits);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(r);
		bn_free(s);
		bn_free(t);
	}
}