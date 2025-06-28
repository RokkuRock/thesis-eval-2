static void ep_mul_sim_endom(ep_t r, const ep_t p, const bn_t k, const ep_t q,
		const bn_t m, const ep_t *t) {
	int i, l, l0, l1, l2, l3, sk0, sk1, sl0, sl1, w, g = 0;
	int8_t naf0[RLC_FP_BITS + 1], naf1[RLC_FP_BITS + 1], *t0, *t1, u;
	int8_t naf2[RLC_FP_BITS + 1], naf3[RLC_FP_BITS + 1], *t2, *t3;
	bn_t n, k0, k1, m0, m1;
	bn_t v1[3], v2[3];
	ep_t v;
	ep_t tab0[1 << (EP_WIDTH - 2)];
	ep_t tab1[1 << (EP_WIDTH - 2)];
	bn_null(n);
	bn_null(k0);
	bn_null(k1);
	bn_null(m0);
	bn_null(m1);
	ep_null(v);
	for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
		ep_null(tab0[i]);
		ep_null(tab1[i]);
	}
	RLC_TRY {
		bn_new(n);
		bn_new(k0);
		bn_new(k1);
		bn_new(m0);
		bn_new(m1);
		ep_new(v);
		for (i = 0; i < 3; i++) {
			bn_null(v1[i]);
			bn_null(v2[i]);
			bn_new(v1[i]);
			bn_new(v2[i]);
		}
		ep_curve_get_ord(n);
		ep_curve_get_v1(v1);
		ep_curve_get_v2(v2);
		bn_rec_glv(k0, k1, k, n, (const bn_t *)v1, (const bn_t *)v2);
		sk0 = bn_sign(k0);
		sk1 = bn_sign(k1);
		bn_abs(k0, k0);
		bn_abs(k1, k1);
		bn_rec_glv(m0, m1, m, n, (const bn_t *)v1, (const bn_t *)v2);
		sl0 = bn_sign(m0);
		sl1 = bn_sign(m1);
		bn_abs(m0, m0);
		bn_abs(m1, m1);
		g = (t == NULL ? 0 : 1);
		if (!g) {
			for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
				ep_new(tab0[i]);
			}
			ep_tab(tab0, p, EP_WIDTH);
			t = (const ep_t *)tab0;
		}
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_new(tab1[i]);
		}
		ep_tab(tab1, q, EP_WIDTH);
		if (g) {
			w = EP_DEPTH;
		} else {
			w = EP_WIDTH;
		}
		l0 = l1 = l2 = l3 = RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, k0, w);
		bn_rec_naf(naf1, &l1, k1, w);
		bn_rec_naf(naf2, &l2, m0, EP_WIDTH);
		bn_rec_naf(naf3, &l3, m1, EP_WIDTH);
		l = RLC_MAX(RLC_MAX(l0, l1), RLC_MAX(l2, l3));
		t0 = naf0 + l - 1;
		t1 = naf1 + l - 1;
		t2 = naf2 + l - 1;
		t3 = naf3 + l - 1;
		if (bn_sign(k) == RLC_NEG) {
			for (i =  0; i < l0; i++) {
				naf0[i] = -naf0[i];
			}
			for (i =  0; i < l1; i++) {
				naf1[i] = -naf1[i];
			}
		}
		if (bn_sign(m) == RLC_NEG) {
			for (i =  0; i < l2; i++) {
				naf2[i] = -naf2[i];
			}
			for (i =  0; i < l3; i++) {
				naf3[i] = -naf3[i];
			}
		}
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--, t0--, t1--, t2--, t3--) {
			ep_dbl(r, r);
			u = *t0;
			if (u > 0) {
				if (sk0 == RLC_POS) {
					ep_add(r, r, t[u / 2]);
				} else {
					ep_sub(r, r, t[u / 2]);
				}
			}
			if (u < 0) {
				if (sk0 == RLC_POS) {
					ep_sub(r, r, t[-u / 2]);
				} else {
					ep_add(r, r, t[-u / 2]);
				}
			}
			u = *t1;
			if (u > 0) {
				ep_psi(v, t[u / 2]);
				if (sk1 == RLC_NEG) {
					ep_neg(v, v);
				}
				ep_add(r, r, v);
			}
			if (u < 0) {
				ep_psi(v, t[-u / 2]);
				if (sk1 == RLC_NEG) {
					ep_neg(v, v);
				}
				ep_sub(r, r, v);
			}
			u = *t2;
			if (u > 0) {
				if (sl0 == RLC_POS) {
					ep_add(r, r, tab1[u / 2]);
				} else {
					ep_sub(r, r, tab1[u / 2]);
				}
			}
			if (u < 0) {
				if (sl0 == RLC_POS) {
					ep_sub(r, r, tab1[-u / 2]);
				} else {
					ep_add(r, r, tab1[-u / 2]);
				}
			}
			u = *t3;
			if (u > 0) {
				ep_psi(v, tab1[u / 2]);
				if (sl1 == RLC_NEG) {
					ep_neg(v, v);
				}
				ep_add(r, r, v);
			}
			if (u < 0) {
				ep_psi(v, tab1[-u / 2]);
				if (sl1 == RLC_NEG) {
					ep_neg(v, v);
				}
				ep_sub(r, r, v);
			}
		}
		ep_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(k0);
		bn_free(k1);
		bn_free(m0);
		bn_free(m1);
		ep_free(v);
		if (!g) {
			for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
				ep_free(tab0[i]);
			}
		}
		for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
			ep_free(tab1[i]);
		}
		for (i = 0; i < 3; i++) {
			bn_free(v1[i]);
			bn_free(v2[i]);
		}
	}
}