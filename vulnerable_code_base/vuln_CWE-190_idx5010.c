int cp_bls_sig(g1_t s, const uint8_t *msg, int len, const bn_t d) {
	g1_t p;
	int result = RLC_OK;
	g1_null(p);
	RLC_TRY {
		g1_new(p);
		g1_map(p, msg, len);
		g1_mul_key(s, p, d);
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		g1_free(p);
	}
	return result;
}