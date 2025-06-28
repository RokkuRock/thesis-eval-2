void fp54_exp_dig(fp54_t c, const fp54_t a, dig_t b) {
	bn_t _b;
	fp54_t t, v;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	if (b == 0) {
		fp54_set_dig(c, 1);
		return;
	}
	bn_null(_b);
	fp54_null(t);
	fp54_null(v);
	RLC_TRY {
		bn_new(_b);
		fp54_new(t);
		fp54_new(v);
		fp54_copy(t, a);
		if (fp54_test_cyc(a)) {
			fp54_inv_cyc(v, a);
			bn_set_dig(_b, b);
			l = RLC_DIG + 1;
			bn_rec_naf(naf, &l, _b, 2);
			for (int i = bn_bits(_b) - 2; i >= 0; i--) {
				fp54_sqr_cyc(t, t);
				u = naf[i];
				if (u > 0) {
					fp54_mul(t, t, a);
				} else if (u < 0) {
					fp54_mul(t, t, v);
				}
			}
		} else {
			for (int i = util_bits_dig(b) - 2; i >= 0; i--) {
				fp54_sqr(t, t);
				if (b & ((dig_t)1 << i)) {
					fp54_mul(t, t, a);
				}
			}
		}
		fp54_copy(c, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(_b);
		fp54_free(t);
		fp54_free(v);
	}
}