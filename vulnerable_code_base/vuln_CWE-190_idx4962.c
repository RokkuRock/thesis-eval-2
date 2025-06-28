void fp48_exp_cyc_sps(fp48_t c, const fp48_t a, const int *b, int len, int sign) {
	int i, j, k, w = len;
    fp48_t t, *u = RLC_ALLOCA(fp48_t, w);
	if (len == 0) {
		RLC_FREE(u);
		fp48_set_dig(c, 1);
		return;
	}
	fp48_null(t);
	RLC_TRY {
		if (u == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		for (i = 0; i < w; i++) {
			fp48_null(u[i]);
			fp48_new(u[i]);
		}
		fp48_new(t);
		fp48_copy(t, a);
		if (b[0] == 0) {
			for (j = 0, i = 1; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp48_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp48_inv_cyc(u[i - 1], t);
				} else {
					fp48_copy(u[i - 1], t);
				}
			}
			fp48_back_cyc_sim(u, u, w - 1);
			fp48_copy(c, a);
			for (i = 0; i < w - 1; i++) {
				fp48_mul(c, c, u[i]);
			}
		} else {
			for (j = 0, i = 0; i < len; i++) {
				k = (b[i] < 0 ? -b[i] : b[i]);
				for (; j < k; j++) {
					fp48_sqr_pck(t, t);
				}
				if (b[i] < 0) {
					fp48_inv_cyc(u[i], t);
				} else {
					fp48_copy(u[i], t);
				}
			}
			fp48_back_cyc_sim(u, u, w);
			fp48_copy(c, u[0]);
			for (i = 1; i < w; i++) {
				fp48_mul(c, c, u[i]);
			}
		}
		if (sign == RLC_NEG) {
			fp48_inv_cyc(c, c);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < w; i++) {
			fp48_free(u[i]);
		}
		fp48_free(t);
		RLC_FREE(u);
	}
}