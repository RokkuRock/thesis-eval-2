void ep_mul_dig(ep_t r, const ep_t p, dig_t k) {
	ep_t t;
	bn_t _k;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	ep_null(t);
	bn_null(_k);
	if (k == 0 || ep_is_infty(p)) {
		ep_set_infty(r);
		return;
	}
	RLC_TRY {
		ep_new(t);
		bn_new(_k);
		bn_set_dig(_k, k);
		l = RLC_DIG + 1;
		bn_rec_naf(naf, &l, _k, 2);
		ep_set_infty(t);
		for (int i = l - 1; i >= 0; i--) {
			ep_dbl(t, t);
			u = naf[i];
			if (u > 0) {
				ep_add(t, t, p);
			} else if (u < 0) {
				ep_sub(t, t, p);
			}
		}
		ep_norm(r, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep_free(t);
		bn_free(_k);
	}
}