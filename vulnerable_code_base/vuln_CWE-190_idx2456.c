void fp_write_bin(uint8_t *bin, int len, const fp_t a) {
	bn_t t;
	bn_null(t);
	if (len != RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(t);
		fp_prime_back(t, a);
		bn_write_bin(bin, len, t);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}