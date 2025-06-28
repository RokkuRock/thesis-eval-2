static void ep_mul_glv_imp(ep_t r, const ep_t p, const bn_t k) {
	int l, l0, l1, i, n0, n1, s0, s1;
	int8_t naf0[RLC_FP_BITS + 1], naf1[RLC_FP_BITS + 1], *t0, *t1;
	bn_t n, _k, k0, k1, v1[3], v2[3];
	ep_t q, t[1 << (EP_WIDTH - 2)];
	bn_null(n);
	bn_null(_k);
	bn_null(k0);
	bn_null(k1);
	ep_null(q);
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		bn_new(k0);
		bn_new(k1);
		ep_new(q);
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		for (i = 0; i < 3; i++) {
			bn_null(v1[i]);
			bn_null(v2[i]);
			bn_new(v1[i]);
			bn_new(v2[i]);
		}
		ep_curve_get_ord(n);
		ep_curve_get_v1(v1);
		ep_curve_get_v2(v2);
		bn_mod(_k, k, n);
		bn_rec_glv(k0, k1, _k, n, (const bn_t *)v1, (const bn_t *)v2);
		s0 = bn_sign(k0);
		s1 = bn_sign(k1);
		bn_abs(k0, k0);
		bn_abs(k1, k1);
		if (s0 == RLC_POS) {
			ep_tab(t, p, EP_WIDTH);
		} else {
			ep_neg(q, p);
			ep_tab(t, q, EP_WIDTH);
		}
		l0 = l1 = RLC_FP_BITS + 1;
		bn_rec_naf(naf0, &l0, k0, EP_WIDTH);
		bn_rec_naf(naf1, &l1, k1, EP_WIDTH);
		l = RLC_MAX(l0, l1);
		t0 = naf0 + l - 1;
		t1 = naf1 + l - 1;
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--, t0--, t1--) {
			ep_dbl(r, r);
			n0 = *t0;
			n1 = *t1;
			if (n0 > 0) {
				ep_add(r, r, t[n0 / 2]);
			}
			if (n0 < 0) {
				ep_sub(r, r, t[-n0 / 2]);
			}
			if (n1 > 0) {
				ep_psi(q, t[n1 / 2]);
				if (s0 != s1) {
					ep_neg(q, q);
				}
				ep_add(r, r, q);
			}
			if (n1 < 0) {
				ep_psi(q, t[-n1 / 2]);
				if (s0 != s1) {
					ep_neg(q, q);
				}
				ep_sub(r, r, q);
			}
		}
		ep_norm(r, r);
		if (bn_sign(_k) == RLC_NEG) {
			ep_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		bn_free(k0);
		bn_free(k1);
		bn_free(n)
		ep_free(q);
		for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
			ep_free(t[i]);
		}
		for (i = 0; i < 3; i++) {
			bn_free(v1[i]);
			bn_free(v2[i]);
		}
	}
}