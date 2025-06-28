void fp24_exp_dig(fp24_t c, const fp24_t a, dig_t b) {
	bn_t _b;
	fp24_t t, v;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	if (b == 0) {
		fp24_set_dig(c, 1);
		return;
	}
	bn_null(_b);
	fp24_null(t);
	fp24_null(v);
	RLC_TRY {
		bn_new(_b);
		fp24_new(t);
		fp24_new(v);
		fp24_copy(t, a);
		if (fp24_test_cyc(a)) {
			fp24_inv_cyc(v, a);
			bn_set_dig(_b, b);
			l = RLC_DIG + 1;
			bn_rec_naf(naf, &l, _b, 2);
			for (int i = bn_bits(_b) - 2; i >= 0; i--) {
				fp24_sqr_cyc(t, t);
				u = naf[i];
				if (u > 0) {
					fp24_mul(t, t, a);
				} else if (u < 0) {
					fp24_mul(t, t, v);
				}
			}
		} else {
			for (int i = util_bits_dig(b) - 2; i >= 0; i--) {
				fp24_sqr(t, t);
				if (b & ((dig_t)1 << i)) {
					fp24_mul(t, t, a);
				}
			}
		}
		fp24_copy(c, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(_b);
		fp24_free(t);
		fp24_free(v);
	}
}