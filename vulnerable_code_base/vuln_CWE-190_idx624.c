static void ep_mul_sim_plain(ep_t r, const ep_t p, const bn_t k, const ep_t q,
		const bn_t m, const ep_t *t) {
	int i, l, l0, l1, w, gen;
	int8_t naf0[RLC_FP_BITS + 1], naf1[RLC_FP_BITS + 1], n0, n1, *u, *v;
	ep_t t0[1 << (EP_WIDTH - 2)];
	ep_t t1[1 << (EP_WIDTH - 2)];
	RLC_TRY {
		gen = (t == NULL ? 0 : 1);
		if (!gen) {
			for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
				ep_null(t0[i]);
				ep_new(t0[i]);
			}
			ep_tab(t0, p, EP_WIDTH);
			t = (const ep_t *)t0;
		}
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_null(t1[i]);
			ep_new(t1[i]);
		}
		ep_tab(t1, q, EP_WIDTH);
		if (gen) {
			w = EP_DEPTH;
		} else {
			w = EP_WIDTH;
		}
		l0 = l1 = RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, k, w);
		bn_rec_naf(naf1, &l1, m, EP_WIDTH);
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
		u = naf0 + l - 1;
		v = naf1 + l - 1;
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--, u--, v--) {
			ep_dbl(r, r);
			n0 = *u;
			n1 = *v;
			if (n0 > 0) {
				ep_add(r, r, t[n0 / 2]);
			}
			if (n0 < 0) {
				ep_sub(r, r, t[-n0 / 2]);
			}
			if (n1 > 0) {
				ep_add(r, r, t1[n1 / 2]);
			}
			if (n1 < 0) {
				ep_sub(r, r, t1[-n1 / 2]);
			}
		}
		ep_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		if (!gen) {
			for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
				ep_free(t0[i]);
			}
		}
		for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
			ep_free(t1[i]);
		}
	}
}