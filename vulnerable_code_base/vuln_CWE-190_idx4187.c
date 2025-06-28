void fp2_exp_cyc(fp2_t c, const fp2_t a, const bn_t b) {
	fp2_t r, s, t[1 << (FP_WIDTH - 2)];
	int i, l;
	int8_t naf[RLC_FP_BITS + 1], *k;
	if (bn_is_zero(b)) {
		return fp2_set_dig(c, 1);
	}
	fp2_null(r);
	fp2_null(s);
	RLC_TRY {
		fp2_new(r);
		fp2_new(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i ++) {
			fp2_null(t[i]);
			fp2_new(t[i]);
		}
#if FP_WIDTH > 2
		fp2_sqr(t[0], a);
		fp2_mul(t[1], t[0], a);
		for (int i = 2; i < (1 << (FP_WIDTH - 2)); i++) {
			fp2_mul(t[i], t[i - 1], t[0]);
		}
#endif
		fp2_copy(t[0], a);
		l = RLC_FP_BITS + 1;
		fp2_set_dig(r, 1);
		bn_rec_naf(naf, &l, b, FP_WIDTH);
		k = naf + l - 1;
		for (i = l - 1; i >= 0; i--, k--) {
			fp2_sqr(r, r);
			if (*k > 0) {
				fp2_mul(r, r, t[*k / 2]);
			}
			if (*k < 0) {
				fp2_inv_cyc(s, t[-*k / 2]);
				fp2_mul(r, r, s);
			}
		}
		if (bn_sign(b) == RLC_NEG) {
			fp2_inv_cyc(c, r);
		} else {
			fp2_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp2_free(r);
		fp2_free(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i++) {
			fp2_free(t[i]);
		}
	}
}