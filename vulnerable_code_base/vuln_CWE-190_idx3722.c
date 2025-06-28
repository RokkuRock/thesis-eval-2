void bn_rec_reg(int8_t *naf, int *len, const bn_t k, int n, int w) {
	int i, l;
	bn_t t;
	dig_t t0, mask;
	int8_t u_i;
	bn_null(t);
	mask = RLC_MASK(w);
	l = RLC_CEIL(n, w - 1);
	if (*len <= l) {
		*len = 0;
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(t);
		bn_abs(t, k);
		memset(naf, 0, *len);
		i = 0;
		if (w == 2) {
			for (i = 0; i < l; i++) {
				u_i = (t->dp[0] & mask) - 2;
				t->dp[0] -= u_i;
				naf[i] = u_i;
				bn_hlv(t, t);
			}
			bn_get_dig(&t0, t);
			naf[i] = t0;
		} else {
			for (i = 0; i < l; i++) {
				u_i = (t->dp[0] & mask) - (1 << (w - 1));
				t->dp[0] -= u_i;
				naf[i] = u_i;
				bn_rsh(t, t, w - 1);
			}
			bn_get_dig(&t0, t);
			naf[i] = t0;
		}
		*len = l + 1;
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}