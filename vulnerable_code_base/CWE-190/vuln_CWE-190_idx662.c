static void ep_mul_naf_imp(ep_t r, const ep_t p, const bn_t k) {
	int i, l;
	int8_t u, naf[RLC_FP_BITS + 2];
	ep_t t[1 << (EP_WIDTH - 2)];
	bn_t _k, n;
	bn_null(n);
	bn_null(_k);
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		ep_curve_get_ord(n);
		bn_mod(_k, k, n);
		ep_tab(t, p, EP_WIDTH);
		l = RLC_FP_BITS + 2;
		bn_rec_naf(naf, &l, _k, EP_WIDTH);
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ep_dbl(r, r);
			u = naf[i];
			if (u > 0) {
				ep_add(r, r, t[u / 2]);
			} else if (u < 0) {
				ep_sub(r, r, t[-u / 2]);
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
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_free(t[i]);
		}
	}
}