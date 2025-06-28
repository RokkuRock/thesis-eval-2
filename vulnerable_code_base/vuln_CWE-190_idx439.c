void ep2_mul_dig(ep2_t r, const ep2_t p, const dig_t k) {
	ep2_t t;
	bn_t _k;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	ep2_null(t);
	bn_null(_k);
	if (k == 0 || ep2_is_infty(p)) {
		ep2_set_infty(r);
		return;
	}
	RLC_TRY {
		ep2_new(t);
		bn_new(_k);
		bn_set_dig(_k, k);
		l = RLC_DIG + 1;
		bn_rec_naf(naf, &l, _k, 2);
		ep2_set_infty(t);
		for (int i = l - 1; i >= 0; i--) {
			ep2_dbl(t, t);
			u = naf[i];
			if (u > 0) {
				ep2_add(t, t, p);
			} else if (u < 0) {
				ep2_sub(t, t, p);
			}
		}
		ep2_norm(r, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep2_free(t);
		bn_free(_k);
	}
}