int bn_size_str(const bn_t a, int radix) {
	int digits = 0;
	bn_t t;
	bn_null(t);
	if (radix < 2 || radix > 64) {
		RLC_THROW(ERR_NO_VALID);
		return 0;
	}
	if (bn_is_zero(a)) {
		return 2;
	}
	if (radix == 2) {
		return bn_bits(a) + (a->sign == RLC_NEG ? 1 : 0) + 1;
	}
	if (a->sign == RLC_NEG) {
		digits++;
	}
	RLC_TRY {
		bn_new(t);
		bn_copy(t, a);
		t->sign = RLC_POS;
		while (!bn_is_zero(t)) {
			bn_div_dig(t, t, (dig_t)radix);
			digits++;
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		bn_free(t);
	}
	return digits + 1;
}