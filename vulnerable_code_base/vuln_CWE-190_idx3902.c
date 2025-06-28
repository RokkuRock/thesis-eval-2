static void eb_mul_sim_plain(eb_t r, const eb_t p, const bn_t k, const eb_t q,
		const bn_t m, const eb_t *t) {
	int i, l, l0, l1, n0, n1, w, g;
	int8_t naf0[RLC_FB_BITS + 1], naf1[RLC_FB_BITS + 1], *_k, *_m;
	eb_t t0[1 << (EB_WIDTH - 2)];
	eb_t t1[1 << (EB_WIDTH - 2)];
	for (i =  0; i < (1 << (EB_WIDTH - 2)); i++) {
		eb_null(t0[i]);
		eb_null(t1[i]);
	}
	RLC_TRY {
		g = (t == NULL ? 0 : 1);
		if (!g) {
			for (i =  0; i < (1 << (EB_WIDTH - 2)); i++) {
				eb_new(t0[i]);
			}
			eb_tab(t0, p, EB_WIDTH);
			t = (const eb_t *)t0;
		}
		for (i =  0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_new(t1[i]);
		}
		eb_tab(t1, q, EB_WIDTH);
		if (g) {
			w = EB_DEPTH;
		} else {
			w = EB_WIDTH;
		}
		l0 = l1 = RLC_FB_BITS + 1;
		bn_rec_naf(naf0, &l0, k, w);
		bn_rec_naf(naf1, &l1, m, EB_WIDTH);
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
		eb_set_infty(r);
		for (i =  l - 1; i >= 0; i--, _k--, _m--) {
			eb_dbl(r, r);
			n0 = *_k;
			n1 = *_m;
			if (n0 > 0) {
				eb_add(r, r, t[n0 / 2]);
			}
			if (n0 < 0) {
				eb_sub(r, r, t[-n0 / 2]);
			}
			if (n1 > 0) {
				eb_add(r, r, t1[n1 / 2]);
			}
			if (n1 < 0) {
				eb_sub(r, r, t1[-n1 / 2]);
			}
		}
		eb_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		if (!g) {
			for (i =  0; i < 1 << (EB_WIDTH - 2); i++) {
				eb_free(t0[i]);
			}
		}
		for (i =  0; i < 1 << (EB_WIDTH - 2); i++) {
			eb_free(t1[i]);
		}
	}
}