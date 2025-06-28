void bn_srt(bn_t c, bn_t a) {
	bn_t h, l, m, t;
	int bits, cmp;
	if (bn_sign(a) == RLC_NEG) {
		RLC_THROW(ERR_NO_VALID);
	}
	bits = bn_bits(a);
	bits += (bits % 2);
	bn_null(h);
	bn_null(l);
	bn_null(m);
	bn_null(t);
	RLC_TRY {
		bn_new(h);
		bn_new(l);
		bn_new(m);
		bn_new(t);
		bn_set_2b(h, bits >> 1);
		bn_set_2b(l, (bits >> 1) - 1);
		do {
			bn_add(m, h, l);
			bn_hlv(m, m);
			bn_sqr(t, m);
			cmp = bn_cmp(t, a);
			bn_sub(t, h, l);
			if (cmp == RLC_GT) {
				bn_copy(h, m);
			} else if (cmp == RLC_LT) {
				bn_copy(l, m);
			}
		} while (bn_cmp_dig(t, 1) == RLC_GT && cmp != RLC_EQ);
		bn_copy(c, m);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(h);
		bn_free(l);
		bn_free(m);
		bn_free(t);
	}
}