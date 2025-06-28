void ep2_mul_sim_trick(ep2_t r, const ep2_t p, const bn_t k, const ep2_t q,
		const bn_t m) {
	ep2_t t0[1 << (EP_WIDTH / 2)];
	ep2_t t1[1 << (EP_WIDTH / 2)];
	ep2_t t[1 << EP_WIDTH];
	bn_t n, _k, _m;
	int l0, l1, w = EP_WIDTH / 2;
	uint8_t w0[2 * RLC_FP_BITS], w1[2 * RLC_FP_BITS];
	bn_null(n);
	bn_null(_k);
	bn_null(_m);
	if (bn_is_zero(k) || ep2_is_infty(p)) {
		ep2_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ep2_is_infty(q)) {
		ep2_mul(r, p, k);
		return;
	}
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		bn_new(_m);
		ep2_curve_get_ord(n);
		bn_mod(_k, k, n);
		bn_mod(_m, m, n);
		for (int i = 0; i < (1 << w); i++) {
			ep2_null(t0[i]);
			ep2_null(t1[i]);
			ep2_new(t0[i]);
			ep2_new(t1[i]);
		}
		for (int i = 0; i < (1 << EP_WIDTH); i++) {
			ep2_null(t[i]);
			ep2_new(t[i]);
		}
		ep2_set_infty(t0[0]);
		ep2_copy(t0[1], p);
		if (bn_sign(k) == RLC_NEG) {
			ep2_neg(t0[1], t0[1]);
		}
		for (int i = 2; i < (1 << w); i++) {
			ep2_add(t0[i], t0[i - 1], t0[1]);
		}
		ep2_set_infty(t1[0]);
		ep2_copy(t1[1], q);
		if (bn_sign(m) == RLC_NEG) {
			ep2_neg(t1[1], t1[1]);
		}
		for (int i = 1; i < (1 << w); i++) {
			ep2_add(t1[i], t1[i - 1], t1[1]);
		}
		for (int i = 0; i < (1 << w); i++) {
			for (int j = 0; j < (1 << w); j++) {
				ep2_add(t[(i << w) + j], t0[i], t1[j]);
			}
		}
#if defined(EP_MIXED)
		ep2_norm_sim(t + 1, t + 1, (1 << (EP_WIDTH)) - 1);
#endif
		l0 = l1 = RLC_CEIL(2 * RLC_FP_BITS, w);
		bn_rec_win(w0, &l0, k, w);
		bn_rec_win(w1, &l1, m, w);
		for (int i = l0; i < l1; i++) {
			w0[i] = 0;
		}
		for (int i = l1; i < l0; i++) {
			w1[i] = 0;
		}
		ep2_set_infty(r);
		for (int i = RLC_MAX(l0, l1) - 1; i >= 0; i--) {
			for (int j = 0; j < w; j++) {
				ep2_dbl(r, r);
			}
			ep2_add(r, r, t[(w0[i] << w) + w1[i]]);
		}
		ep2_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		bn_free(_m);
		for (int i = 0; i < (1 << w); i++) {
			ep2_free(t0[i]);
			ep2_free(t1[i]);
		}
		for (int i = 0; i < (1 << EP_WIDTH); i++) {
			ep2_free(t[i]);
		}
	}
}