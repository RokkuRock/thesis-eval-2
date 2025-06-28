void ep2_mul_sim_lot(ep2_t r, const ep2_t p[], const bn_t k[], int n) {
	const int len = RLC_FP_BITS + 1;
	int i, j, m, l, _l[4];
	bn_t _k[4], q, x;
	int8_t ptr, *naf = RLC_ALLOCA(int8_t, 4 * n * len);
	if (n == 0) {
		ep2_set_infty(r);
		return;
	}
	bn_null(q);
	bn_null(x);
	if (n <= 10) {
		ep2_t *_p = RLC_ALLOCA(ep2_t, 4 * n);
		RLC_TRY {
			if (naf == NULL || _p == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			bn_new(q);
			bn_new(x);
			for (j = 0; j < 4; j++) {
				bn_null(_k[j]);
				bn_new(_k[j]);
				for (i = 0; i < n; i++) {
					ep2_null(_p[4*i + j]);
					ep2_new(_p[4*i + j]);
				}
			}
			l = 0;
			ep2_curve_get_ord(q);
			fp_prime_get_par(x);
			for (i = 0; i < n; i++) {
				ep2_norm(_p[4*i], p[i]);
				ep2_frb(_p[4*i + 1], _p[4*i], 1);
				ep2_frb(_p[4*i + 2], _p[4*i + 1], 1);
				ep2_frb(_p[4*i + 3], _p[4*i + 2], 1);
				bn_mod(_k[0], k[i], q);
				bn_rec_frb(_k, 4, _k[0], x, q, ep_curve_is_pairf() == EP_BN);
				for (j = 0; j < 4; j++) {
					_l[j] = len;
					bn_rec_naf(&naf[(4*i + j)*len], &_l[j], _k[j], 2);
					if (bn_sign(_k[j]) == RLC_NEG) {
						ep2_neg(_p[4*i + j], _p[4*i + j]);
					}
					l = RLC_MAX(l, _l[j]);
				}
			}
			ep2_set_infty(r);
			for (i = l - 1; i >= 0; i--) {
				ep2_dbl(r, r);
				for (j = 0; j < n; j++) {
					for (m = 0; m < 4; m++) {
						if (naf[(4*j + m)*len + i] > 0) {
							ep2_add(r, r, _p[4*j + m]);
						}
						if (naf[(4*j + m)*len + i] < 0) {
							ep2_sub(r, r, _p[4*j + m]);
						}
					}
				}
			}
			ep2_norm(r, r);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			bn_free(x);
			for (j = 0; j < 4; j++) {
				bn_free(_k[j]);
				for (i = 0; i < n; i++) {
					ep2_free(_p[4*i + j]);
				}
			}
			RLC_FREE(_p);
			RLC_FREE(naf);
		}
	} else {
		const int w = RLC_MAX(2, util_bits_dig(n) - 2), c = (1 << (w - 2));
		ep2_t s, t, u, v, *_p = RLC_ALLOCA(ep2_t, 4 * c);
		ep2_null(s);
		ep2_null(t);
		ep2_null(u);
		ep2_null(v);
		RLC_TRY {
			if (naf == NULL || _p == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			bn_new(q);
			bn_new(x);
			ep2_new(s);
			ep2_new(t);
			ep2_new(u);
			ep2_new(v);
			for (i = 0; i < 4; i++) {
				bn_null(_k[i]);
				bn_new(_k[i]);
				for (j = 0; j < c; j++) {
					ep2_null(_p[i*c + j]);
					ep2_new(_p[i*c + j]);
					ep2_set_infty(_p[i*c + j]);
				}
			}
			l = 0;
			ep2_curve_get_ord(q);
			fp_prime_get_par(x);
			for (i = 0; i < n; i++) {
				bn_mod(_k[0], k[i], q);
				bn_rec_frb(_k, 4, _k[0], x, q, ep_curve_is_pairf() == EP_BN);
				for (j = 0; j < 4; j++) {
					_l[j] = len;
					bn_rec_naf(&naf[(4*i + j)*len], &_l[j], _k[j], w);
					if (bn_sign(_k[j]) == RLC_NEG) {
						for (m = 0; m < _l[j]; m++) {
							naf[(4*i + j)*len + m] = -naf[(4*i + j)*len + m];
						}
					}
					l = RLC_MAX(l, _l[j]);
				}
			}
			ep2_set_infty(s);
			for (i = l - 1; i >= 0; i--) {
				for (j = 0; j < n; j++) {
					for (m = 0; m < 4; m++) {
						ptr = naf[(4*j + m)*len + i];
						if (ptr != 0) {
							ep2_copy(t, p[j]);
							if (ptr < 0) {
								ptr = -ptr;
								ep2_neg(t, t);
							}
							ep2_add(_p[m*c + (ptr/2)], _p[m*c + (ptr/2)], t);
						}
					}
				}
				ep2_set_infty(t);
				for (m = 3; m >= 0; m--) {
					ep2_frb(t, t, 1);
					ep2_set_infty(u);
					ep2_set_infty(v);
					for (j = c - 1; j >= 0; j--) {
						ep2_add(u, u, _p[m*c + j]);
						if (j == 0) {
							ep2_dbl(v, v);
						}
						ep2_add(v, v, u);
						ep2_set_infty(_p[m*c + j]);
					}
					ep2_add(t, t, v);
				}
				ep2_dbl(s, s);
				ep2_add(s, s, t);
			}
			ep2_norm(r, s);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			bn_free(x);
			ep2_free(s);
			ep2_free(t);
			ep2_free(u);
			ep2_free(v);
			for (i = 0; i < 4; i++) {
				bn_free(_k[i]);
				for (j = 0; j < c; j++) {
					ep2_free(_p[i*c + j]);
				}
			}
			RLC_FREE(_p);
			RLC_FREE(naf);
		}
	}
}