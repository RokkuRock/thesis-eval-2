void bn_rec_naf(int8_t *naf, int *len, const bn_t k, int w) {
	int i, l;
	bn_t t;
	dig_t t0, mask;
	int8_t u_i;
	if (*len < (bn_bits(k) + 1)) {
		*len = 0;
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	bn_null(t);
	RLC_TRY {
		bn_new(t);
		bn_abs(t, k);
		mask = RLC_MASK(w);
		l = (1 << w);
		memset(naf, 0, *len);
		i = 0;
		if (w == 2) {
			while (!bn_is_zero(t)) {
				if (!bn_is_even(t)) {
					bn_get_dig(&t0, t);
					u_i = 2 - (t0 & mask);
					if (u_i < 0) {
						bn_add_dig(t, t, -u_i);
					} else {
						bn_sub_dig(t, t, u_i);
					}
					*naf = u_i;
				} else {
					*naf = 0;
				}
				bn_hlv(t, t);
				i++;
				naf++;
			}
		} else {
			while (!bn_is_zero(t)) {
				if (!bn_is_even(t)) {
					bn_get_dig(&t0, t);
					u_i = t0 & mask;
					if (u_i > l / 2) {
						u_i = (int8_t)(u_i - l);
					}
					if (u_i < 0) {
						bn_add_dig(t, t, -u_i);
					} else {
						bn_sub_dig(t, t, u_i);
					}
					*naf = u_i;
				} else {
					*naf = 0;
				}
				bn_hlv(t, t);
				i++;
				naf++;
			}
		}
		*len = i;
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}