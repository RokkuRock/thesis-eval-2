int fp_size_str(const fp_t a, int radix) {
	bn_t t;
	int digits = 0;
	bn_null(t);
	RLC_TRY {
		bn_new(t);
		fp_prime_back(t, a);
		digits = bn_size_str(t, radix);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
	return digits;
}