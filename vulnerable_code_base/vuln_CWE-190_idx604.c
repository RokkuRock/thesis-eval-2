static void ep2_mul_sim_endom(ep2_t r, const ep2_t p, const bn_t k, ep2_t q,
		const bn_t m) {
	int i, j, l, _l[4];
	bn_t _k[4], _m[4], n, u;
	int8_t naf0[4][RLC_FP_BITS + 1];
	int8_t naf1[4][RLC_FP_BITS + 1];
	ep2_t _p[4], _q[4];
	bn_null(n);
	bn_null(u);
	RLC_TRY {
		bn_new(n);
		bn_new(u);
		for (i = 0; i < 4; i++) {
			bn_null(_k[i]);
			bn_new(_k[i]);
			bn_null(_m[i]);
			bn_new(_m[i]);
			ep2_null(_p[i]);
			ep2_null(_q[i]);
			ep2_new(_p[i]);
			ep2_new(_q[i]);
		}
		ep2_norm(_p[0], p);
		ep2_frb(_p[1], _p[0], 1);
		ep2_frb(_p[2], _p[1], 1);
		ep2_frb(_p[3], _p[2], 1);
		ep2_norm(_q[0], q);
		ep2_frb(_q[1], _q[0], 1);
		ep2_frb(_q[2], _q[1], 1);
		ep2_frb(_q[3], _q[2], 1);
		ep2_curve_get_ord(n);
		fp_prime_get_par(u);
		bn_mod(_k[0], k, n);
		bn_rec_frb(_k, 4, _k[0], u, n, ep_curve_is_pairf() == EP_BN);
		bn_mod(_m[0], m, n);
		bn_rec_frb(_m, 4, _m[0], u, n, ep_curve_is_pairf() == EP_BN);
		l = 0;
		for (i = 0; i < 4; i++) {
			_l[i] = RLC_FP_BITS + 1;
			bn_rec_naf(naf0[i], &_l[i], _k[i], 2);
			if (bn_sign(_k[i]) == RLC_NEG) {
				ep2_neg(_p[i], _p[i]);
			}
			l = RLC_MAX(l, _l[i]);
			_l[i] = RLC_FP_BITS + 1;
			bn_rec_naf(naf1[i], &_l[i], _m[i], 2);
			if (bn_sign(_m[i]) == RLC_NEG) {
				ep2_neg(_q[i], _q[i]);
			}
			l = RLC_MAX(l, _l[i]);
		}
		ep2_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			ep2_dbl(r, r);
			for (j = 0; j < 4; j++) {
				if (naf0[j][i] > 0) {
					ep2_add(r, r, _p[j]);
				}
				if (naf0[j][i] < 0) {
					ep2_sub(r, r, _p[j]);
				}
				if (naf1[j][i] > 0) {
					ep2_add(r, r, _q[j]);
				}
				if (naf1[j][i] < 0) {
					ep2_sub(r, r, _q[j]);
				}
			}
		}
		ep2_norm(r, r);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		bn_free(n);
		bn_free(u);
		for (i = 0; i < 4; i++) {
			bn_free(_k[i]);
			bn_free(_m[i]);
			ep2_free(_p[i]);
			ep2_free(_q[i]);
		}
	}
}