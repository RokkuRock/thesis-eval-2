static void ep2_mul_naf_imp(ep2_t r, const ep2_t p, const bn_t k) {
	int l, i, n;
	int8_t naf[RLC_FP_BITS + 1];
	ep2_t t[1 << (EP_WIDTH - 2)];
	RLC_TRY {
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep2_null(t[i]);
			ep2_new(t[i]);
		}
		ep2_tab(t, p, EP_WIDTH);
		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, EP_WIDTH);
		ep2_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ep2_dbl(r, r);
			n = naf[i];
			if (n > 0) {
				ep2_add(r, r, t[n / 2]);
			}
			if (n < 0) {
				ep2_sub(r, r, t[-n / 2]);
			}
		}
		ep2_norm(r, r);
		if (bn_sign(k) == RLC_NEG) {
			ep2_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep2_free(t[i]);
		}
	}
}