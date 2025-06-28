void ed_mul_dig(ed_t r, const ed_t p, dig_t k) {
	ed_t t;
	bn_t _k;
	int8_t u, naf[RLC_DIG + 1];
	int l;
	ed_null(t);
	bn_null(_k);
	if (k == 0 || ed_is_infty(p)) {
		ed_set_infty(r);
		return;
	}
	RLC_TRY {
		ed_new(t);
		bn_new(_k);
		bn_set_dig(_k, k);
		l = RLC_DIG + 1;
		bn_rec_naf(naf, &l, _k, 2);
		ed_set_infty(t);
		for (int i = l - 1; i >= 0; i--) {
			ed_dbl(t, t);
			u = naf[i];
			if (u > 0) {
				ed_add(t, t, p);
			} else if (u < 0) {
				ed_sub(t, t, p);
			}
		}
		ed_norm(r, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ed_free(t);
		bn_free(_k);
	}
}