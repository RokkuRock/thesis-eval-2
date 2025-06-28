void fp24_exp_cyc_sps(fp24_t c, const fp24_t a, const int *b, int len, int sign) {
	int i, j, k, w = len;
    fp24_t t, *u = RLC_ALLOCA(fp24_t, w);
	if (len == 0) {
		RLC_FREE(u);
		fp24_set_dig(c, 1);
		return;
	}
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
		fp24_copy(t, a);
		if (b[0] == 0) {
			for (j = 0, i = 1; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp24_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp24_inv_cyc(u[i - 1], t);
				} else {
					fp24_copy(u[i - 1], t);
				}
			}
			fp24_back_cyc_sim(u, u, w - 1);
			fp24_copy(c, a);
			for (i = 0; i < w - 1; i++) {
				fp24_mul(c, c, u[i]);
			}
		} else {
			for (j = 0, i = 0; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp24_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp24_inv_cyc(u[i], t);
				} else {
					fp24_copy(u[i], t);
				}
			}
			fp24_back_cyc_sim(u, u, w);
			fp24_copy(c, u[0]);
			for (i = 1; i < w; i++) {
				fp24_mul(c, c, u[i]);
			}
		}
		if (sign == RLC_NEG) {
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