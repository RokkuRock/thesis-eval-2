void ed_mul_sim_lot(ed_t r, const ed_t p[], const bn_t k[], int n) {
	int i, j, l, *_l = RLC_ALLOCA(int, n);
	ed_t *_p = RLC_ALLOCA(ed_t, n);
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
			ed_null(_p[i]);
			ed_new(_p[i]);
		}
		for (i = 0; i < n; i++) {
			_l[i] = l;
			ed_norm(_p[i], p[i]);
			bn_rec_naf(&naf[i*l], &_l[i], k[i], 2);
			if (bn_sign(k[i]) == RLC_NEG) {
				ed_neg(_p[i], _p[i]);
			}
		}
		ed_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ed_dbl(r, r);
			for (j = 0; j < n; j++) {
				if (naf[j*l + i] > 0) {
					ed_add(r, r, _p[j]);
				}
				if (naf[j*l + i] < 0) {
					ed_sub(r, r, _p[j]);
				}
			}
		}
		ed_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		for (i = 0; i < n; i++) {
			ed_free(_p[i]);
		}
		RLC_FREE(_l);
		RLC_FREE(_p);
		RLC_FREE(naf);
	}
}