void fp2_exp_cyc_sim(fp2_t e, const fp2_t a, const bn_t b, const fp2_t c, const bn_t d) {
	int i, l, n0, n1, l0, l1;
	int8_t naf0[RLC_FP_BITS + 1], naf1[RLC_FP_BITS + 1], *_k, *_m;
	fp2_t r, t0[1 << (EP_WIDTH - 2)];
	fp2_t s, t1[1 << (EP_WIDTH - 2)];
	if (bn_is_zero(b)) {
		return fp2_exp_cyc(e, c, d);
	}
	if (bn_is_zero(d)) {
		return fp2_exp_cyc(e, a, b);
	}
	fp2_null(r);
	fp2_null(s);
	RLC_TRY {
		fp2_new(r);
		fp2_new(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i ++) {
			fp2_null(t0[i]);
			fp2_null(t1[i]);
			fp2_new(t0[i]);
			fp2_new(t1[i]);
		}
#if FP_WIDTH > 2
		fp2_sqr(t0[0], a);
		fp2_mul(t0[1], t0[0], a);
		for (int i = 2; i < (1 << (FP_WIDTH - 2)); i++) {
			fp2_mul(t0[i], t0[i - 1], t0[0]);
		}
		fp2_sqr(t1[0], c);
		fp2_mul(t1[1], t1[0], c);
		for (int i = 2; i < (1 << (FP_WIDTH - 2)); i++) {
			fp2_mul(t1[i], t1[i - 1], t1[0]);
		}
#endif
		fp2_copy(t0[0], a);
		fp2_copy(t1[0], c);
		l0 = l1 = RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, b, FP_WIDTH);
		bn_rec_naf(naf1, &l1, d, FP_WIDTH);
		l = RLC_MAX(l0, l1);
		if (bn_sign(b) == RLC_NEG) {
			for (i = 0; i < l0; i++) {
				naf0[i] = -naf0[i];
			}
		}
		if (bn_sign(d) == RLC_NEG) {
			for (i = 0; i < l1; i++) {
				naf1[i] = -naf1[i];
			}
		}
		_k = naf0 + l - 1;
		_m = naf1 + l - 1;
		fp2_set_dig(r, 1);
		for (i = l - 1; i >= 0; i--, _k--, _m--) {
			fp2_sqr(r, r);
			n0 = *_k;
			n1 = *_m;
			if (n0 > 0) {
				fp2_mul(r, r, t0[n0 / 2]);
			}
			if (n0 < 0) {
				fp2_inv_cyc(s, t0[-n0 / 2]);
				fp2_mul(r, r, s);
			}
			if (n1 > 0) {
				fp2_mul(r, r, t1[n1 / 2]);
			}
			if (n1 < 0) {
				fp2_inv_cyc(s, t1[-n1 / 2]);
				fp2_mul(r, r, s);
			}
		}
		fp2_copy(e, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp2_free(r);
		fp2_free(s);
		for (i = 0; i < (1 << (FP_WIDTH - 2)); i++) {
			fp2_free(t0[i]);
			fp2_free(t1[i]);
		}
	}
}