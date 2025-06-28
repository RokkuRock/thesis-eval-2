void ep_mul_sim_lot_plain(ep_t r, const ep_t p[], const bn_t k[], int n) {
	int i, j, l, *_l = RLC_ALLOCA(int, n);
	ep_t *_p = RLC_ALLOCA(ep_t, n);
	int8_t *naf = NULL;
	RLC_TRY {
		l = 0;
		for (i = 0; i < n; i++) {
			l = RLC_MAX(l, bn_bits(k[i]) + 1);
		}
		naf = RLC_ALLOCA(int8_t, n * l);
		if (naf == NULL || _p == NULL || _l == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		for (i = 0; i < n; i++) {
			ep_null(_p[i]);
			ep_new(_p[i]);
		}
		for (i = 0; i < n; i++) {
			_l[i] = l;
			ep_norm(_p[i], p[i]);
			bn_rec_naf(&naf[i*l], &_l[i], k[i], 2);
			if (bn_sign(k[i]) == RLC_NEG) {
				ep_neg(_p[i], _p[i]);
			}
		}
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ep_dbl(r, r);
			for (j = 0; j < n; j++) {
				if (naf[j*l + i] > 0) {
					ep_add(r, r, _p[j]);
				}
				if (naf[j*l + i] < 0) {
					ep_sub(r, r, _p[j]);
				}
			}
		}
		ep_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		for (i = 0; i < n; i++) {
			ep_free(_p[i]);
		}
		RLC_FREE(_l);
		RLC_FREE(_p);
		RLC_FREE(naf);
	}
}