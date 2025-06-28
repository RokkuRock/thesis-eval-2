void fp12_exp_cyc(fp12_t c, const fp12_t a, const bn_t b) {
	int i, j, k, l, w = bn_ham(b);
	if (bn_is_zero(b)) {
		return fp12_set_dig(c, 1);
	}
	if ((bn_bits(b) > RLC_DIG) && ((w << 3) > bn_bits(b))) {
		int _l[4];
		int8_t naf[4][RLC_FP_BITS + 1];
		fp12_t t[4];
		bn_t _b[4], n, u;
		bn_null(n);
		bn_null(u);
		RLC_TRY {
			bn_new(n);
			bn_new(u);
			for (i = 0; i < 4; i++) {
				bn_null(_b[i]);
				bn_new(_b[i]);
				fp12_null(t[i]);
				fp12_new(t[i]);
			}
			ep_curve_get_ord(n);
			fp_prime_get_par(u);
			bn_rec_frb(_b, 4, b, u, n, ep_curve_is_pairf() == EP_BN);
			if (ep_curve_is_pairf()) {
				fp12_copy(t[0], a);
				fp12_frb(t[1], t[0], 1);
				fp12_frb(t[2], t[1], 1);
				fp12_frb(t[3], t[2], 1);
				l = 0;
				for (i = 0; i < 4; i++) {
					if (bn_sign(_b[i]) == RLC_NEG) {
						fp12_inv_cyc(t[i], t[i]);
					}
					_l[i] = RLC_FP_BITS + 1;
					bn_rec_naf(naf[i], &_l[i], _b[i], 2);
					l = RLC_MAX(l, _l[i]);
				}
				fp12_set_dig(c, 1);
				for (i = l - 1; i >= 0; i--) {
					fp12_sqr_cyc(c, c);
					for (j = 0; j < 4; j++) {
						if (naf[j][i] > 0) {
							fp12_mul(c, c, t[j]);
						}
						if (naf[j][i] < 0) {
							fp12_inv_cyc(t[j], t[j]);
							fp12_mul(c, c, t[j]);
							fp12_inv_cyc(t[j], t[j]);
						}
					}
				}
			} else {
				fp12_copy(t[0], a);
				for (i = bn_bits(b) - 2; i >= 0; i--) {
					fp12_sqr_cyc(t[0], t[0]);
					if (bn_get_bit(b, i)) {
						fp12_mul(t[0], t[0], a);
					}
				}
				fp12_copy(c, t[0]);
				if (bn_sign(b) == RLC_NEG) {
					fp12_inv_cyc(c, c);
				}
			}
		}
		RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		}
		RLC_FINALLY {
			bn_free(n);
			bn_free(u);
			for (i = 0; i < 4; i++) {
				bn_free(_b[i]);
				fp12_free(t[i]);
			}
		}
	} else {
		fp12_t t, *u = RLC_ALLOCA(fp12_t, w);
		fp12_null(t);
		RLC_TRY {
			if (u == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			for (i = 0; i < w; i++) {
				fp12_null(u[i]);
				fp12_new(u[i]);
			}
			fp12_new(t);
			j = 0;
			fp12_copy(t, a);
			for (i = 1; i < bn_bits(b); i++) {
				fp12_sqr_pck(t, t);
				if (bn_get_bit(b, i)) {
					fp12_copy(u[j++], t);
				}
			}
			if (!bn_is_even(b)) {
				j = 0;
				k = w - 1;
			} else {
				j = 1;
				k = w;
			}
			fp12_back_cyc_sim(u, u, k);
			if (!bn_is_even(b)) {
				fp12_copy(c, a);
			} else {
				fp12_copy(c, u[0]);
			}
			for (i = j; i < k; i++) {
				fp12_mul(c, c, u[i]);
			}
			if (bn_sign(b) == RLC_NEG) {
				fp12_inv_cyc(c, c);
			}
		}
		RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		}
		RLC_FINALLY {
			for (i = 0; i < w; i++) {
				fp12_free(u[i]);
			}
			fp12_free(t);
			RLC_FREE(u);
		}
	}
}