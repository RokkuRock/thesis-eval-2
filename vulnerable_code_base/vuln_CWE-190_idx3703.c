static void ep4_mul_glv_imp(ep4_t r, const ep4_t p, const bn_t k) {
	int sign, i, j, l, _l[8];
	bn_t n, _k[8], u, v;
	int8_t naf[8][RLC_FP_BITS + 1];
	ep4_t q[8];
	bn_null(n);
	bn_null(u);
	bn_null(v);
	RLC_TRY {
		bn_new(n);
		bn_new(u);
		bn_new(v);
		for (i = 0; i < 8; i++) {
			bn_null(_k[i]);
			ep4_null(q[i]);
			bn_new(_k[i]);
			ep4_new(q[i]);
		}
        bn_abs(v, k);
		ep4_curve_get_ord(n);
        if (bn_cmp_abs(v, n) == RLC_GT) {
            bn_mod(v, v, n);
        }
		fp_prime_get_par(u);
		sign = bn_sign(u);
        bn_abs(u, u);
		ep4_norm(q[0], p);
		for (i = 0; i < 8; i++) {
			bn_mod(_k[i], v, u);
			bn_div(v, v, u);
			if ((sign == RLC_NEG) && (i % 2 != 0)) {
				bn_neg(_k[i], _k[i]);
			}
            if (bn_sign(k) == RLC_NEG) {
                bn_neg(_k[i], _k[i]);
            }
            if (i > 0) {
                ep4_frb(q[i], q[i - 1], 1);
            }
		}
        l = 0;
		for (i = 0; i < 8; i++) {
			if (bn_sign(_k[i]) == RLC_NEG) {
				ep4_neg(q[i], q[i]);
			}
			_l[i] = RLC_FP_BITS + 1;
			bn_rec_naf(naf[i], &_l[i], _k[i], 2);
            l = RLC_MAX(l, _l[i]);
		}
		ep4_set_infty(r);
		for (j = l - 1; j >= 0; j--) {
			ep4_dbl(r, r);
			for (i = 0; i < 8; i++) {
				if (naf[i][j] > 0) {
					ep4_add(r, r, q[i]);
				}
				if (naf[i][j] < 0) {
					ep4_sub(r, r, q[i]);
				}
			}
		}
		ep4_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
        bn_free(u);
        bn_free(v);
		for (i = 0; i < 8; i++) {
			bn_free(_k[i]);
			ep4_free(q[i]);
		}
	}
}