void fp8_exp_cyc(fp8_t c, const fp8_t a, const bn_t b) {
	fp8_t r, s, t[1 << (FP_WIDTH - 2)];
	int i, l;
	int8_t naf[RLC_FP_BITS + 1], *k;
	if (bn_is_zero(b)) {
		return fp8_set_dig(c, 1);
	}
	fp8_null(r);
	fp8_null(s);
	RLC_TRY {
		fp8_new(r);
		fp8_new(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i ++) {
			fp8_null(t[i]);
			fp8_new(t[i]);
		}
#if FP_WIDTH > 2
		fp8_sqr_cyc(t[0], a);
		fp8_mul(t[1], t[0], a);
		for (int i = 2; i < (1 << (FP_WIDTH - 2)); i++) {
			fp8_mul(t[i], t[i - 1], t[0]);
		}
#endif
		fp8_copy(t[0], a);
		l = RLC_FP_BITS + 1;
		fp8_set_dig(r, 1);
		bn_rec_naf(naf, &l, b, FP_WIDTH);
		k = naf + l - 1;
		for (i = l - 1; i >= 0; i--, k--) {
			fp8_sqr_cyc(r, r);
			if (*k > 0) {
				fp8_mul(r, r, t[*k / 2]);
			}
			if (*k < 0) {
				fp8_inv_cyc(s, t[-*k / 2]);
				fp8_mul(r, r, s);
			}
		}
		if (bn_sign(b) == RLC_NEG) {
			fp8_inv_cyc(c, r);
		} else {
			fp8_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp8_free(r);
		fp8_free(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i++) {
			fp8_free(t[i]);
		}
	}
}