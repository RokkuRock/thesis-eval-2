void fp12_exp_cyc_sps(fp12_t c, const fp12_t a, const int *b, int len, int sign) {
	int i, j, k, w = len;
    fp12_t t, *u = RLC_ALLOCA(fp12_t, w);
	if (len == 0) {
		RLC_FREE(u);
		fp12_set_dig(c, 1);
		return;
	}
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
		fp12_copy(t, a);
		if (b[0] == 0) {
			for (j = 0, i = 1; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp12_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp12_inv_cyc(u[i - 1], t);
				} else {
					fp12_copy(u[i - 1], t);
				}
			}
			fp12_back_cyc_sim(u, u, w - 1);
			fp12_copy(c, a);
			for (i = 0; i < w - 1; i++) {
				fp12_mul(c, c, u[i]);
			}
		} else {
			for (j = 0, i = 0; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp12_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp12_inv_cyc(u[i], t);
				} else {
					fp12_copy(u[i], t);
				}
			}
			fp12_back_cyc_sim(u, u, w);
			fp12_copy(c, u[0]);
			for (i = 1; i < w; i++) {
				fp12_mul(c, c, u[i]);
			}
		}
		if (sign == RLC_NEG) {
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