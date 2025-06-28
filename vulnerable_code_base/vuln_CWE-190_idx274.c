void eb_mul_sim_trick(eb_t r, const eb_t p, const bn_t k, const eb_t q,
		const bn_t m) {
	eb_t t0[1 << (EB_WIDTH / 2)], t1[1 << (EB_WIDTH / 2)], t[1 << EB_WIDTH];
	int l0, l1, w = EB_WIDTH / 2;
	uint8_t w0[RLC_FB_BITS], w1[RLC_FB_BITS];
	bn_t n;
	bn_null(n);
	if (bn_is_zero(k) || eb_is_infty(p)) {
		eb_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || eb_is_infty(q)) {
		eb_mul(r, p, k);
		return;
	}
	RLC_TRY {
		bn_new(n);
		eb_curve_get_ord(n);
		for (int i = 0; i < (1 << w); i++) {
			eb_null(t0[i]);
			eb_null(t1[i]);
			eb_new(t0[i]);
			eb_new(t1[i]);
		}
		for (int i = 0; i < (1 << EB_WIDTH); i++) {
			eb_null(t[i]);
			eb_new(t[i]);
		}
		eb_set_infty(t0[0]);
		eb_copy(t0[1], p);
		if (bn_sign(k) == RLC_NEG) {
			eb_neg(t0[1], t0[1]);
		}
		for (int i = 2; i < (1 << w); i++) {
			eb_add(t0[i], t0[i - 1], t0[1]);
		}
		eb_set_infty(t1[0]);
		eb_copy(t1[1], q);
		if (bn_sign(m) == RLC_NEG) {
			eb_neg(t1[1], t1[1]);
		}
		for (int i = 2; i < (1 << w); i++) {
			eb_add(t1[i], t1[i - 1], t1[1]);
		}
		for (int i = 0; i < (1 << w); i++) {
			for (int j = 0; j < (1 << w); j++) {
				eb_add(t[(i << w) + j], t0[i], t1[j]);
			}
		}
#if EB_WIDTH > 2 && defined(EB_MIXED)
		eb_norm_sim(t + 1, (const eb_t *)(t + 1), (1 << EB_WIDTH) - 1);
#endif
		l0 = l1 = RLC_CEIL(RLC_FB_BITS + 1, w);
		bn_rec_win(w0, &l0, k, w);
		bn_rec_win(w1, &l1, m, w);
		for (int i = l0; i < l1; i++) {
			w0[i] = 0;
		}
		for (int i = l1; i < l0; i++) {
			w1[i] = 0;
		}
		eb_set_infty(r);
		for (int i = RLC_MAX(l0, l1) - 1; i >= 0; i--) {
			for (int j = 0; j < w; j++) {
				eb_dbl(r, r);
			}
			eb_add(r, r, t[(w0[i] << w) + w1[i]]);
		}
		eb_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		for (int i = 0; i < (1 << w); i++) {
			eb_free(t0[i]);
			eb_free(t1[i]);
		}
		for (int i = 0; i < (1 << EB_WIDTH); i++) {
			eb_free(t[i]);
		}
	}
}