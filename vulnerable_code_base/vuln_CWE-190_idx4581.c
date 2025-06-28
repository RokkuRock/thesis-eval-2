static void ep4_mul_naf_imp(ep4_t r, const ep4_t p, const bn_t k) {
	int l, i, n;
	int8_t naf[RLC_FP_BITS + 1];
	ep4_t t[1 << (EP_WIDTH - 2)];
	RLC_TRY {
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep4_null(t[i]);
			ep4_new(t[i]);
		}
		ep4_tab(t, p, EP_WIDTH);
		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, EP_WIDTH);
		ep4_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ep4_dbl(r, r);
			n = naf[i];
			if (n > 0) {
				ep4_add(r, r, t[n / 2]);
			}
			if (n < 0) {
				ep4_sub(r, r, t[-n / 2]);
			}
		}
		ep4_norm(r, r);
		if (bn_sign(k) == RLC_NEG) {
			ep4_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep4_free(t[i]);
		}
	}
}