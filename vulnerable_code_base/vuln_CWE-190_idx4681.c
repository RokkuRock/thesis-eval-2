void ed_mul_sim_trick(ed_t r, const ed_t p, const bn_t k, const ed_t q,
		const bn_t m) {
	ed_t t0[1 << (ED_WIDTH / 2)], t1[1 << (ED_WIDTH / 2)], t[1 << ED_WIDTH];
	bn_t n;
	int l0, l1, w = ED_WIDTH / 2;
	uint8_t w0[RLC_FP_BITS + 1], w1[RLC_FP_BITS + 1];
	bn_null(n);
	if (bn_is_zero(k) || ed_is_infty(p)) {
		ed_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ed_is_infty(q)) {
		ed_mul(r, p, k);
		return;
	}
	RLC_TRY {
		bn_new(n);
		ed_curve_get_ord(n);
		for (int i = 0; i < (1 << w); i++) {
			ed_null(t0[i]);
			ed_null(t1[i]);
			ed_new(t0[i]);
			ed_new(t1[i]);
		}
		for (int i = 0; i < (1 << ED_WIDTH); i++) {
			ed_null(t[i]);
			ed_new(t[i]);
		}
		ed_set_infty(t0[0]);
		ed_copy(t0[1], p);
		if (bn_sign(k) == RLC_NEG) {
			ed_neg(t0[1], t0[1]);
		}
		for (int i = 2; i < (1 << w); i++) {
			ed_add(t0[i], t0[i - 1], t0[1]);
		}
		ed_set_infty(t1[0]);
		ed_copy(t1[1], q);
		if (bn_sign(m) == RLC_NEG) {
			ed_neg(t1[1], t1[1]);
		}
		for (int i = 1; i < (1 << w); i++) {
			ed_add(t1[i], t1[i - 1], t1[1]);
		}
		for (int i = 0; i < (1 << w); i++) {
			for (int j = 0; j < (1 << w); j++) {
				ed_add(t[(i << w) + j], t0[i], t1[j]);
			}
		}
#if defined(ED_MIXED)
		ed_norm_sim(t + 1, (const ed_t *)t + 1, (1 << (ED_WIDTH)) - 1);
#endif
		l0 = l1 = RLC_CEIL(RLC_FP_BITS, w);
		bn_rec_win(w0, &l0, k, w);
		bn_rec_win(w1, &l1, m, w);
		for (int i = l0; i < l1; i++) {
			w0[i] = 0;
		}
		for (int i = l1; i < l0; i++) {
			w1[i] = 0;
		}
		ed_set_infty(r);
		for (int i = RLC_MAX(l0, l1) - 1; i >= 0; i--) {
			for (int j = 0; j < w; j++) {
				ed_dbl(r, r);
			}
			ed_add(r, r, t[(w0[i] << w) + w1[i]]);
		}
		ed_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		for (int i = 0; i < (1 << w); i++) {
			ed_free(t0[i]);
			ed_free(t1[i]);
		}
		for (int i = 0; i < (1 << ED_WIDTH); i++) {
			ed_free(t[i]);
		}
	}
}