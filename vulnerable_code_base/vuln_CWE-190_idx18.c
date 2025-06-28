void fp48_exp_dig(fp48_t c, const fp48_t a, dig_t b) {
	bn_t _b;
	fp48_t t, v;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	if (b == 0) {
		fp48_set_dig(c, 1);
		return;
	}
	bn_null(_b);
	fp48_null(t);
	fp48_null(v);
	RLC_TRY {
		bn_new(_b);
		fp48_new(t);
		fp48_new(v);
		fp48_copy(t, a);
		if (fp48_test_cyc(a)) {
			fp48_inv_cyc(v, a);
			bn_set_dig(_b, b);
			l = RLC_DIG + 1;
			bn_rec_naf(naf, &l, _b, 2);
			for (int i = bn_bits(_b) - 2; i >= 0; i--) {
				fp48_sqr_cyc(t, t);
				u = naf[i];
				if (u > 0) {
					fp48_mul(t, t, a);
				} else if (u < 0) {
					fp48_mul(t, t, v);
				}
			}
		} else {
			for (int i = util_bits_dig(b) - 2; i >= 0; i--) {
				fp48_sqr(t, t);
				if (b & ((dig_t)1 << i)) {
					fp48_mul(t, t, a);
				}
			}
		}
		fp48_copy(c, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(_b);
		fp48_free(t);
		fp48_free(v);
	}
}