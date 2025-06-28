void fp12_exp_dig(fp12_t c, const fp12_t a, dig_t b) {
	bn_t _b;
	fp12_t t, v;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	if (b == 0) {
		fp12_set_dig(c, 1);
		return;
	}
	bn_null(_b);
	fp12_null(t);
	fp12_null(v);
	RLC_TRY {
		bn_new(_b);
		fp12_new(t);
		fp12_new(v);
		fp12_copy(t, a);
		if (fp12_test_cyc(a)) {
			fp12_inv_cyc(v, a);
			bn_set_dig(_b, b);
			l = RLC_DIG + 1;
			bn_rec_naf(naf, &l, _b, 2);
			for (int i = bn_bits(_b) - 2; i >= 0; i--) {
				fp12_sqr_cyc(t, t);
				u = naf[i];
				if (u > 0) {
					fp12_mul(t, t, a);
				} else if (u < 0) {
					fp12_mul(t, t, v);
				}
			}
		} else {
			for (int i = util_bits_dig(b) - 2; i >= 0; i--) {
				fp12_sqr(t, t);
				if (b & ((dig_t)1 << i)) {
					fp12_mul(t, t, a);
				}
			}
		}
		fp12_copy(c, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(_b);
		fp12_free(t);
		fp12_free(v);
	}
}