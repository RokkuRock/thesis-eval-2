void fp54_exp_cyc_sps(fp54_t c, const fp54_t a, const int *b, int len, int sign) {
	int i, j, k, w = len;
    fp54_t t, *u = RLC_ALLOCA(fp54_t, w);
	if (len == 0) {
		RLC_FREE(u);
		fp54_set_dig(c, 1);
		return;
	}
	fp54_null(t);
	RLC_TRY {
		if (u == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		for (i = 0; i < w; i++) {
			fp54_null(u[i]);
			fp54_new(u[i]);
		}
		fp54_new(t);
		fp54_copy(t, a);
		if (b[0] == 0) {
			for (j = 0, i = 1; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp54_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp54_inv_cyc(u[i - 1], t);
				} else {
					fp54_copy(u[i - 1], t);
				}
			}
			fp54_back_cyc_sim(u, u, w - 1);
			fp54_copy(c, a);
			for (i = 0; i < w - 1; i++) {
				fp54_mul(c, c, u[i]);
			}
		} else {
			for (j = 0, i = 0; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp54_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp54_inv_cyc(u[i], t);
				} else {
					fp54_copy(u[i], t);
				}
			}
			fp54_back_cyc_sim(u, u, w);
			fp54_copy(c, u[0]);
			for (i = 1; i < w; i++) {
				fp54_mul(c, c, u[i]);
			}
		}
		if (sign == RLC_NEG) {
			fp54_inv_cyc(c, c);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < w; i++) {
			fp54_free(u[i]);
		}
		fp54_free(t);
		RLC_FREE(u);
	}
}