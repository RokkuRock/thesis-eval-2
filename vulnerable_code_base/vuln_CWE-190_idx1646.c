void fp24_exp_cyc(fp24_t c, const fp24_t a, const bn_t b) {
	int i, j, k, w = bn_ham(b);
	if (bn_is_zero(b)) {
		fp24_set_dig(c, 1);
		return;
	}
	if ((bn_bits(b) > RLC_DIG) && ((w << 3) > bn_bits(b))) {
		int l, _l[8];
		int8_t naf[8][RLC_FP_BITS + 1];
		fp24_t t[8];
		bn_t _b[8], n, x;
		bn_null(n);
		bn_null(x);
		RLC_TRY {
			bn_new(n);
			bn_new(x);
			for (i = 0; i < 8; i++) {
				bn_null(_b[i]);
				bn_new(_b[i]);
				fp24_null(t[i]);
				fp24_new(t[i]);
			}
			ep_curve_get_ord(n);
			fp_prime_get_par(x);
			bn_rec_frb(_b, 8, b, x, n, ep_curve_is_pairf() == EP_BN);
			if (ep_curve_is_pairf()) {
				l = 0;
				fp24_copy(t[0], a);
				for (i = 0; i < 8; i++) {
					_l[i] = RLC_FP_BITS + 1;
					bn_rec_naf(naf[i], &_l[i], _b[i], 2);
					l = RLC_MAX(l, _l[i]);
					if (i > 0) {
						fp24_frb(t[i], t[i - 1], 1);
					}
				}
				for (i = 0; i < 8; i++) {
					if (bn_sign(_b[i]) == RLC_NEG) {
						fp24_inv_cyc(t[i], t[i]);
					}
				}
				fp24_set_dig(c, 1);
				for (i = l - 1; i >= 0; i--) {
					fp24_sqr_cyc(c, c);
					for (j = 0; j < 8; j++) {
						if (naf[j][i] > 0) {
							fp24_mul(c, c, t[j]);
						}
						if (naf[j][i] < 0) {
							fp24_inv_cyc(t[j], t[j]);
							fp24_mul(c, c, t[j]);
							fp24_inv_cyc(t[j], t[j]);
						}
					}
				}
			} else {
				fp24_copy(t[0], a);
				for (i = bn_bits(b) - 2; i >= 0; i--) {
					fp24_sqr_cyc(t[0], t[0]);
					if (bn_get_bit(b, i)) {
						fp24_mul(t[0], t[0], a);
					}
				}
				fp24_copy(c, t[0]);
				if (bn_sign(b) == RLC_NEG) {
					fp24_inv_cyc(c, c);
				}
			}
		}
		RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		}
		RLC_FINALLY {
			bn_free(n);
			bn_free(x);
			for (i = 0; i < 8; i++) {
				bn_free(_b[i]);
				fp24_free(t[i]);
			}
		}
	} else {
		fp24_t t, *u = RLC_ALLOCA(fp24_t, w);
		fp24_null(t);
		RLC_TRY {
			if (u == NULL) {
				RLC_THROW(ERR_NO_MEMORY);
			}
			for (i = 0; i < w; i++) {
				fp24_null(u[i]);
				fp24_new(u[i]);
			}
			fp24_new(t);
			j = 0;
			fp24_copy(t, a);
			for (i = 1; i < bn_bits(b); i++) {
				fp24_sqr_pck(t, t);
				if (bn_get_bit(b, i)) {
					fp24_copy(u[j++], t);
				}
			}
			if (!bn_is_even(b)) {
				j = 0;
				k = w - 1;
			} else {
				j = 1;
				k = w;
			}
			fp24_back_cyc_sim(u, u, k);
			if (!bn_is_even(b)) {
				fp24_copy(c, a);
			} else {
				fp24_copy(c, u[0]);
			}
			for (i = j; i < k; i++) {
				fp24_mul(c, c, u[i]);
			}
			if (bn_sign(b) == RLC_NEG) {
				fp24_inv_cyc(c, c);
			}
		}
		RLC_CATCH_ANY {
			RLC_THROW(ERR_CAUGHT);
		}
		RLC_FINALLY {
			for (i = 0; i < w; i++) {
				fp24_free(u[i]);
			}
			fp24_free(t);
			RLC_FREE(u);
		}
	}
}