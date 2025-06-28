void ep_mul_sim_lot_endom(ep_t r, const ep_t p[], const bn_t k[], int n) {
	const int len = RLC_FP_BITS + 1;
	int i, j, m, l, _l[2], sk;
	bn_t _k[2], q, v1[3], v2[3];
	int8_t ptr, *naf = RLC_ALLOCA(int8_t, 2 * n * len);
	bn_null(q);
	if (n <= 10) {
		ep_t *_p = RLC_ALLOCA(ep_t, 2 * n);
		RLC_TRY {
			if (naf == NULL || _p == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			bn_new(q);
			for (j = 0; j < 2; j++) {
				bn_null(_k[j]);
				bn_new(_k[j]);
			}
			for (i = 0; i < 2 * n; i++) {
				ep_null(_p[i]);
				ep_new(_p[i]);
			}
			for (i = 0; i < 3; i++) {
				bn_null(v1[i]);
				bn_null(v2[i]);
				bn_new(v1[i]);
				bn_new(v2[i]);
			}
			l = 0;
			ep_curve_get_ord(q);
			ep_curve_get_v1(v1);
			ep_curve_get_v2(v2);
			for (i = 0; i < n; i++) {
				ep_norm(_p[2*i], p[i]);
				ep_psi(_p[2*i + 1], _p[2*i]);
				bn_mod(_k[0], k[i], q);
				sk = bn_sign(_k[0]);
				bn_rec_glv(_k[0], _k[1], _k[0], q, (const bn_t *)v1, (const bn_t *)v2);
				if (sk == RLC_NEG) {
					bn_neg(_k[0], _k[0]);
					bn_neg(_k[1], _k[1]);
				}
				for (j = 0; j < 2; j++) {
					_l[j] = len;
					bn_rec_naf(&naf[(2*i + j)*len], &_l[j], _k[j], 2);
					if (bn_sign(_k[j]) == RLC_NEG) {
						ep_neg(_p[2*i + j], _p[2*i + j]);
					}
					l = RLC_MAX(l, _l[j]);
				}
			}
			ep_set_infty(r);
			for (i = l - 1; i >= 0; i--) {
				ep_dbl(r, r);
				for (j = 0; j < n; j++) {
					for (m = 0; m < 2; m++) {
						if (naf[(2*j + m)*len + i] > 0) {
							ep_add(r, r, _p[2*j + m]);
						}
						if (naf[(2*j + m)*len + i] < 0) {
							ep_sub(r, r, _p[2*j + m]);
						}
					}
				}
			}
			ep_norm(r, r);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			bn_free(_k[0]);
			bn_free(_k[1]);
			for (i = 0; i < 2 * n; i++) {
				ep_free(_p[i]);
			}
			RLC_FREE(_p);
			RLC_FREE(naf);
			for (i = 0; i < 3; i++) {
				bn_free(v1[i]);
				bn_free(v2[i]);
			}
		}
	} else {
		const int w = RLC_MAX(2, util_bits_dig(n) - 2), c = (1 << (w - 2));
		ep_t s, t, u, v, *_p = RLC_ALLOCA(ep_t, 2 * c);
		ep_null(s);
		ep_null(t);
		ep_null(u);
		ep_null(v);
		RLC_TRY {
			if (naf == NULL || _p == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			bn_new(q);
			ep_new(s);
			ep_new(t);
			ep_new(u);
			ep_new(v);
			for (i = 0; i < 2; i++) {
				bn_null(_k[i]);
				bn_new(_k[i]);
				for (j = 0; j < c; j++) {
					ep_null(_p[i*c + j]);
					ep_new(_p[i*c + j]);
					ep_set_infty(_p[i*c + j]);
				}
			}
			for (i = 0; i < 3; i++) {
				bn_null(v1[i]);
				bn_null(v2[i]);
				bn_new(v1[i]);
				bn_new(v2[i]);
			}
			l = 0;
			ep_curve_get_ord(q);
			ep_curve_get_v1(v1);
			ep_curve_get_v2(v2);
			for (i = 0; i < n; i++) {
				bn_mod(_k[0], k[i], q);
				sk = bn_sign(_k[0]);
				bn_rec_glv(_k[0], _k[1], _k[0], q, (const bn_t *)v1, (const bn_t *)v2);
				if (sk == RLC_NEG) {
					bn_neg(_k[0], _k[0]);
					bn_neg(_k[1], _k[1]);
				}
				for (j = 0; j < 2; j++) {
					_l[j] = len;
					bn_rec_naf(&naf[(2*i + j)*len], &_l[j], _k[j], w);
					if (bn_sign(_k[j]) == RLC_NEG) {
						for (m = 0; m < _l[j]; m++) {
							naf[(2*i + j)*len + m] = -naf[(2*i + j)*len + m];
						}
					}
					l = RLC_MAX(l, _l[j]);
				}
			}
			ep_set_infty(s);
			for (i = l - 1; i >= 0; i--) {
				for (j = 0; j < n; j++) {
					for (m = 0; m < 2; m++) {
						ptr = naf[(2*j + m)*len + i];
						if (ptr != 0) {
							ep_copy(t, p[j]);
							if (ptr < 0) {
								ptr = -ptr;
								ep_neg(t, t);
							}
							ep_add(_p[m*c + (ptr >> 1)], _p[m*c + (ptr >> 1)], t);
						}
					}
				}
				ep_set_infty(t);
				for (m = 1; m >= 0; m--) {
					ep_psi(t, t);
					ep_set_infty(u);
					ep_set_infty(v);
					for (j = c - 1; j >= 0; j--) {
						ep_add(u, u, _p[m*c + j]);
						if (j == 0) {
							ep_dbl(v, v);
						}
						ep_add(v, v, u);
						ep_set_infty(_p[m*c + j]);
					}
					ep_add(t, t, v);
				}
				ep_dbl(s, s);
				ep_add(s, s, t);
			}
			ep_norm(r, s);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			ep_free(s);
			ep_free(t);
			ep_free(u);
			ep_free(v);
			for (i = 0; i < 2; i++) {
				bn_free(_k[i]);
				for (j = 0; j < c; j++) {
					ep_free(_p[i*c + j]);
				}
			}
			RLC_FREE(_p);
			RLC_FREE(naf);
			for (i = 0; i < 3; i++) {
				bn_free(v1[i]);
				bn_free(v2[i]);
			}
		}
	}
}