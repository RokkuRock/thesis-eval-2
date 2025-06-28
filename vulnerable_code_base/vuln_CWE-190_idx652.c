void bn_rec_glv(bn_t k0, bn_t k1, const bn_t k, const bn_t n, const bn_t *v1,
		const bn_t *v2) {
	bn_t t, b1, b2;
	int r1, r2, bits;
	bn_null(b1);
	bn_null(b2);
	bn_null(t);
	RLC_TRY {
		bn_new(b1);
		bn_new(b2);
		bn_new(t);
		bn_abs(t, k);
		bits = bn_bits(n);
		bn_mul(b1, t, v1[0]);
		r1 = bn_get_bit(b1, bits);
		bn_rsh(b1, b1, bits + 1);
		bn_add_dig(b1, b1, r1);
		bn_mul(b2, t, v2[0]);
		r2 = bn_get_bit(b2, bits);
		bn_rsh(b2, b2, bits + 1);
		bn_add_dig(b2, b2, r2);
		bn_mul(k0, b1, v1[1]);
		bn_mul(k1, b2, v2[1]);
		bn_add(k0, k0, k1);
		bn_sub(k0, t, k0);
		bn_mul(k1, b1, v1[2]);
		bn_mul(t, b2, v2[2]);
		bn_add(k1, k1, t);
		bn_neg(k1, k1);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(b1);
		bn_free(b2);
		bn_free(t);
	}
}