static void ep2_mul_glv_imp(ep2_t r, const ep2_t p, const bn_t k) {
	int i, j, l, _l[4];
	bn_t n, _k[4], u;
	int8_t naf[4][RLC_FP_BITS + 1];
	ep2_t q[4];
	bn_null(n);
	bn_null(u);
	RLC_TRY {
		bn_new(n);
		bn_new(u);
		for (i = 0; i < 4; i++) {
			bn_null(_k[i]);
			ep2_null(q[i]);
			bn_new(_k[i]);
			ep2_new(q[i]);
		}
		ep2_curve_get_ord(n);
		fp_prime_get_par(u);
		bn_mod(_k[0], k, n);
		bn_rec_frb(_k, 4, _k[0], u, n, ep_curve_is_pairf() == EP_BN);
		ep2_norm(q[0], p);
		ep2_frb(q[1], q[0], 1);
		ep2_frb(q[2], q[1], 1);
		ep2_frb(q[3], q[2], 1);
		l = 0;
		for (i = 0; i < 4; i++) {
			if (bn_sign(_k[i]) == RLC_NEG) {
				ep2_neg(q[i], q[i]);
			}
			_l[i] = RLC_FP_BITS + 1;
			bn_rec_naf(naf[i], &_l[i], _k[i], 2);
			l = RLC_MAX(l, _l[i]);
		}
		ep2_set_infty(r);
		for (j = l - 1; j >= 0; j--) {
			ep2_dbl(r, r);
			for (i = 0; i < 4; i++) {
				if (naf[i][j] > 0) {
					ep2_add(r, r, q[i]);
				}
				if (naf[i][j] < 0) {
					ep2_sub(r, r, q[i]);
				}
			}
		}
		ep2_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(u);
		for (i = 0; i < 4; i++) {
			bn_free(_k[i]);
			ep2_free(q[i]);
		}
	}
}