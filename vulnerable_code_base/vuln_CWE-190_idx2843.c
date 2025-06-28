static void ed_mul_sim_plain(ed_t r, const ed_t p, const bn_t k, const ed_t q,
		const bn_t m, const ed_t *t) {
	int i, l, l0, l1, n0, n1, w, gen;
	int8_t naf0[RLC_FP_BITS + 1], naf1[RLC_FP_BITS + 1], *_k, *_m;
	ed_t t0[1 << (ED_WIDTH - 2)];
	ed_t t1[1 << (ED_WIDTH - 2)];
	RLC_TRY {
		gen = (t == NULL ? 0 : 1);
		if (!gen) {
			for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
				ed_null(t0[i]);
				ed_new(t0[i]);
			}
			ed_tab(t0, p, ED_WIDTH);
			t = (const ed_t *)t0;
		}
		for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
			ed_null(t1[i]);
			ed_new(t1[i]);
		}
		ed_tab(t1, q, ED_WIDTH);
		if (gen) {
			w = ED_DEPTH;
		} else {
			w = ED_WIDTH;
		}
		l0 = l1 = RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, k, w);
		bn_rec_naf(naf1, &l1, m, ED_WIDTH);
		l = RLC_MAX(l0, l1);
		if (bn_sign(k) == RLC_NEG) {
			for (i =  0; i < l0; i++) {
				naf0[i] = -naf0[i];
			}
		}
		if (bn_sign(m) == RLC_NEG) {
			for (i =  0; i < l1; i++) {
				naf1[i] = -naf1[i];
			}
		}
		_k = naf0 + l - 1;
		_m = naf1 + l - 1;
		ed_set_infty(r);
		for (i = l - 1; i >= 0; i--, _k--, _m--) {
			ed_dbl(r, r);
			n0 = *_k;
			n1 = *_m;
			if (n0 > 0) {
				ed_add(r, r, t[n0 / 2]);
			}
			if (n0 < 0) {
				ed_sub(r, r, t[-n0 / 2]);
			}
			if (n1 > 0) {
				ed_add(r, r, t1[n1 / 2]);
			}
			if (n1 < 0) {
				ed_sub(r, r, t1[-n1 / 2]);
			}
		}
		ed_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		if (!gen) {
			for (i = 0; i < 1 << (ED_WIDTH - 2); i++) {
				ed_free(t0[i]);
			}
		}
		for (i = 0; i < 1 << (ED_WIDTH - 2); i++) {
			ed_free(t1[i]);
		}
	}
}