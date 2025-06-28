void bn_rec_rtnaf(int8_t *tnaf, int *len, const bn_t k, int8_t u, int m, int w) {
	int i, l;
	bn_t tmp, r0, r1;
	int8_t beta[64], gama[64];
	uint8_t t_w;
	dig_t t0, t1, mask;
	int s, t, u_i;
	bn_null(r0);
	bn_null(r1);
	bn_null(tmp);
	if (*len < (bn_bits(k) + 1)) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(r0);
		bn_new(r1);
		bn_new(tmp);
		memset(tnaf, 0, *len);
		bn_rec_tnaf_get(&t_w, beta, gama, u, w);
		bn_abs(tmp, k);
		bn_rec_tnaf_mod(r0, r1, tmp, u, m);
		mask = RLC_MASK(w);
		l = RLC_CEIL(m + 2, (w - 1));
		i = 0;
		while (i < l) {
			if (w == 2) {
				t0 = r0->dp[0];
				if (bn_sign(r0) == RLC_NEG) {
					t0 = (1 << w) - t0;
				}
				t1 = r1->dp[0];
				if (bn_sign(r1) == RLC_NEG) {
					t1 = (1 << w) - t1;
				}
				u_i = ((t0 - 2 * t1) & mask) - 2;
				tnaf[i++] = u_i;
				if (u_i < 0) {
					bn_add_dig(r0, r0, -u_i);
				} else {
					bn_sub_dig(r0, r0, u_i);
				}
			} else {
				t0 = r0->dp[0];
				if (bn_sign(r0) == RLC_NEG) {
					t0 = (1 << w) - t0;
				}
				t1 = r1->dp[0];
				if (bn_sign(r1) == RLC_NEG) {
					t1 = (1 << w) - t1;
				}
				u_i = ((t0 + t_w * t1) & mask) - (1 << (w - 1));
				if (u_i < 0) {
					tnaf[i++] = u_i;
					u_i = (int8_t)(-u_i >> 1);
					t = -beta[u_i];
					s = -gama[u_i];
				} else {
					tnaf[i++] = u_i;
					u_i = (int8_t)(u_i >> 1);
					t = beta[u_i];
					s = gama[u_i];
				}
				if (t > 0) {
					bn_sub_dig(r0, r0, t);
				} else {
					bn_add_dig(r0, r0, -t);
				}
				if (s > 0) {
					bn_sub_dig(r1, r1, s);
				} else {
					bn_add_dig(r1, r1, -s);
				}
			}
			for (int j = 0; j < (w - 1); j++) {
				bn_hlv(tmp, r0);
				if (u == -1) {
					bn_sub(r0, r1, tmp);
				} else {
					bn_add(r0, r1, tmp);
				}
				bn_copy(r1, tmp);
				r1->sign = tmp->sign ^ 1;
			}
		}
		s = r0->dp[0];
		t = r1->dp[0];
		if (bn_sign(r0) == RLC_NEG) {
			s = -s;
		}
		if (bn_sign(r1) == RLC_NEG) {
			t = -t;
		}
		if (s != 0 && t != 0) {
			for (int j = 0; j < (1 << (w - 2)); j++) {
				if (beta[j] == s && gama[j] == t) {
					tnaf[i++] = 2 * j + 1;
					break;
				}
			}
			for (int j = 0; j < (1 << (w - 2)); j++) {
				if (beta[j] == -s && gama[j] == -t) {
					tnaf[i++] = -(2 * j + 1);
					break;
				}
			}
		} else {
			if (t != 0) {
				tnaf[i++] = t;
			} else {
				tnaf[i++] = s;
			}
		}
		*len = i;
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(r0);
		bn_free(r1);
		bn_free(tmp);
	}
}