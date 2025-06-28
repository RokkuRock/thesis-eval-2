void ep4_mul_sim_lot(ep4_t r, const ep4_t p[], const bn_t k[], int n) {
	const int len = RLC_FP_BITS + 1;
	int i, j, m, l, *_l = RLC_ALLOCA(int, 8 * n);
	bn_t _k[8], q, x;
	int8_t *naf = RLC_ALLOCA(int8_t, 8 * n * len);
	bn_null(q);
	bn_null(x);
	if (n <= 10) {
		ep4_t *_p = RLC_ALLOCA(ep4_t, 8 * n);
		RLC_TRY {
			bn_new(q);
			bn_new(x);
			for (j = 0; j < 8; j++) {
				bn_null(_k[j]);
				bn_new(_k[j]);
				for (i = 0; i < n; i++) {
					ep4_null(_p[8*i + j]);
					ep4_new(_p[8*i + j]);
				}
			}
			for (int i = 0; i < n; i++) {
				ep4_norm(_p[8*i], p[i]);
				ep4_frb(_p[8*i + 1], _p[8*i], 1);
				ep4_frb(_p[8*i + 2], _p[8*i + 1], 1);
				ep4_frb(_p[8*i + 3], _p[8*i + 2], 1);
				ep4_frb(_p[8*i + 4], _p[8*i + 3], 1);
				ep4_frb(_p[8*i + 5], _p[8*i + 4], 1);
				ep4_frb(_p[8*i + 6], _p[8*i + 5], 1);
				ep4_frb(_p[8*i + 7], _p[8*i + 6], 1);
			}
			ep_curve_get_ord(q);
			fp_prime_get_par(x);
			l = 0;
			for (i = 0; i < n; i++) {
				bn_rec_frb(_k, 8, k[i], q, x, ep_curve_is_pairf() == EP_BN);
				for (j = 0; j < 8; j++) {
					_l[8*i + j] = len;
					bn_rec_naf(&naf[(8*i + j)*len], &_l[8*i + j], _k[j], 2);
					if (bn_sign(_k[j]) == RLC_NEG) {
						ep4_neg(_p[8*i + j], _p[8*i + j]);
					}
					l = RLC_MAX(l, _l[8*i + j]);
				}
			}
			for (i = 0; i < n; i++) {
				for (j = 0; j < 8; j++) {
					for (m = _l[8*i + j]; m < l; m++) {
						naf[(8*i + j)*len + m] = 0;
					}
				}
			}
			ep4_set_infty(r);
			for (i = l - 1; i >= 0; i--) {
				ep4_dbl(r, r);
				for (j = 0; j < n; j++) {
					for (m = 0; m < 8; m++) {
						if (naf[(8*j + m)*len + i] > 0) {
							ep4_add(r, r, _p[8*j + m]);
						}
						if (naf[(8*j + m)*len + i] < 0) {
							ep4_sub(r, r, _p[8*j + m]);
						}
					}
				}
			}
			ep4_norm(r, r);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			bn_free(x);
			for (j = 0; j < 8; j++) {
				bn_free(_k[j]);
				for (i = 0; i < n; i++) {
					ep4_free(_p[8*i + j]);
				}
			}
			RLC_FREE(_l);
			RLC_FREE(_p);
			RLC_FREE(naf);
		}
	} else {
		const int w = RLC_MAX(2, util_bits_dig(n) - 2), c = (1 << (w - 2));
		ep4_t s, t, u, v, *_p = RLC_ALLOCA(ep4_t, 8 * c);
		int8_t ptr;
		ep4_null(s);
		ep4_null(t);
		ep4_null(u);
		ep4_null(v);
		RLC_TRY {
			bn_new(q);
			bn_new(x);
			ep4_new(s);
			ep4_new(t);
			ep4_new(u);
			ep4_new(v);
			for (i = 0; i < 8; i++) {
				bn_null(_k[i]);
				bn_new(_k[i]);
				for (j = 0; j < c; j++) {
					ep4_null(_p[i*c + j]);
					ep4_new(_p[i*c + j]);
					ep4_set_infty(_p[i*c + j]);
				}
			}
			ep_curve_get_ord(q);
			fp_prime_get_par(x);
			l = 0;
			for (i = 0; i < n; i++) {
				bn_rec_frb(_k, 8, k[i], q, x, ep_curve_is_pairf() == EP_BN);
				for (j = 0; j < 8; j++) {
					_l[8*i + j] = len;
					bn_rec_naf(&naf[(8*i + j)*len], &_l[8*i + j], _k[j], w);
					l = RLC_MAX(l, _l[8*i + j]);
				}
			}
			for (i = 0; i < n; i++) {
				for (j = 0; j < 8; j++) {
					for (m = _l[8*i + j]; m < l; m++) {
						naf[(8*i + j)*len + m] = 0;
					}
				}
			}
			ep4_set_infty(s);
			for (i = l - 1; i >= 0; i--) {
				for (j = 0; j < n; j++) {
					for (m = 0; m < 8; m++) {
						ptr = naf[(8*j + m)*len + i];
						if (ptr != 0) {
							ep4_copy(t, p[j]);
							if (ptr < 0) {
								ptr = -ptr;
								ep4_neg(t, t);
							}
							if (bn_sign(_k[m]) == RLC_NEG) {
								ep4_neg(t, t);
							}
							ep4_add(_p[m*c + (ptr/2)], _p[m*c + (ptr/2)], t);
						}
					}
				}
				ep4_set_infty(t);
				for (m = 3; m >= 0; m--) {
					ep4_frb(t, t, 1);
					ep4_set_infty(u);
					ep4_set_infty(v);
					for (j = c - 1; j >= 0; j--) {
						ep4_add(u, u, _p[m*c + j]);
						if (j == 0) {
							ep4_dbl(v, v);
						}
						ep4_add(v, v, u);
						ep4_set_infty(_p[m*c + j]);
					}
					ep4_add(t, t, v);
				}
				ep4_dbl(s, s);
				ep4_add(s, s, t);
			}
			ep4_norm(r, s);
		} RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		} RLC_FINALLY {
			bn_free(q);
			bn_free(x);
			ep4_free(s);
			ep4_free(t);
			ep4_free(u);
			ep4_free(v);
			for (i = 0; i < 8; i++) {
				bn_free(_k[i]);
				for (j = 0; j < c; j++) {
					ep4_free(_p[i*c + j]);
				}
			}
			RLC_FREE(_l);
			RLC_FREE(_p);
			RLC_FREE(naf);
		}
	}
}