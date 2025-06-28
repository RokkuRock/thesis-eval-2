int bn_is_prime_rabin(const bn_t a) {
	bn_t t, n1, y, r;
	int i, s, j, result, b, tests = 0, cmp2;
	tests = 0;
	result = 1;
	bn_null(t);
	bn_null(n1);
	bn_null(y);
	bn_null(r);
	cmp2 = bn_cmp_dig(a, 2);
	if (cmp2 == RLC_LT) {
		return 0;
	}
	if (cmp2 == RLC_EQ) {
		return 1;
	}
	if (bn_is_even(a) == 1) {
		return 0;
	}
	RLC_TRY {
		b = bn_bits(a);
		if (b >= 1300) {
			tests = 2;
		} else if (b >= 850) {
			tests = 3;
		} else if (b >= 650) {
			tests = 4;
		} else if (b >= 550) {
			tests = 5;
		} else if (b >= 450) {
			tests = 6;
		} else if (b >= 400) {
			tests = 7;
		} else if (b >= 350) {
			tests = 8;
		} else if (b >= 300) {
			tests = 9;
		} else if (b >= 250) {
			tests = 12;
		} else if (b >= 200) {
			tests = 15;
		} else if (b >= 150) {
			tests = 18;
		} else {
			tests = 27;
		}
		bn_new(t);
		bn_new(n1);
		bn_new(y);
		bn_new(r);
		bn_sub_dig(n1, a, 1);
		bn_copy(r, n1);
		s = 0;
		while (bn_is_even(r)) {
			s++;
			bn_rsh(r, r, 1);
		}
		for (i = 0; i < tests; i++) {
			bn_set_dig(t, primes[i]);
			if( bn_cmp(t, n1) != RLC_LT ) {
				result = 1;
				break;
			}
#if BN_MOD != PMERS
			bn_mxp(y, t, r, a);
#else
			bn_exp(y, t, r, a);
#endif
			if (bn_cmp_dig(y, 1) != RLC_EQ && bn_cmp(y, n1) != RLC_EQ) {
				j = 1;
				while ((j <= (s - 1)) && bn_cmp(y, n1) != RLC_EQ) {
					bn_sqr(y, y);
					bn_mod(y, y, a);
					if (bn_cmp_dig(y, 1) == RLC_EQ) {
						result = 0;
						break;
					}
					++j;
				}
				if (bn_cmp(y, n1) != RLC_EQ) {
					result = 0;
					break;
				}
			}
		}
	}
	RLC_CATCH_ANY {
		result = 0;
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(r);
		bn_free(y);
		bn_free(n1);
		bn_free(t);
	}
	return result;
}