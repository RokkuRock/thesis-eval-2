void fp_write_str(char *str, int len, const fp_t a, int radix) {
	bn_t t;
	bn_null(t);
	RLC_TRY {
		bn_new(t);
		fp_prime_back(t, a);
		bn_write_str(str, len, t, radix);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}