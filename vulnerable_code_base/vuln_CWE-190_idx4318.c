static void ep4_mul_sim_plain(ep4_t r, const ep4_t p, const bn_t k,
		const ep4_t q, const bn_t m, ep4_t *t) {
	int i, l, l0, l1, n0, n1, w, gen;
	int8_t naf0[2 * RLC_FP_BITS + 1], naf1[2 * RLC_FP_BITS + 1], *_k, *_m;
	ep4_t t0[1 << (EP_WIDTH - 2)];
	ep4_t t1[1 << (EP_WIDTH - 2)];
	RLC_TRY {
		gen = (t == NULL ? 0 : 1);
		if (!gen) {
			for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
				ep4_null(t0[i]);
				ep4_new(t0[i]);
			}
			ep4_tab(t0, p, EP_WIDTH);
			t = (ep4_t *)t0;
		}
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep4_null(t1[i]);
			ep4_new(t1[i]);
		}
		ep4_tab(t1, q, EP_WIDTH);
		if (gen) {
			w = EP_DEPTH;
		} else {
			w = EP_WIDTH;
		}
		l0 = l1 = 2 * RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, k, w);
		bn_rec_naf(naf1, &l1, m, EP_WIDTH);
		l = RLC_MAX(l0, l1);
		_k = naf0 + l - 1;
		_m = naf1 + l - 1;
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
		ep4_set_infty(r);
		for (i = l - 1; i >= 0; i--, _k--, _m--) {
			ep4_dbl(r, r);
			n0 = *_k;
			n1 = *_m;
			if (n0 > 0) {
				ep4_add(r, r, t[n0 / 2]);
			}
			if (n0 < 0) {
				ep4_sub(r, r, t[-n0 / 2]);
			}
			if (n1 > 0) {
				ep4_add(r, r, t1[n1 / 2]);
			}
			if (n1 < 0) {
				ep4_sub(r, r, t1[-n1 / 2]);
			}
		}
		ep4_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		if (!gen) {
			for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
				ep4_free(t0[i]);
			}
		}
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep4_free(t1[i]);
		}
	}
}