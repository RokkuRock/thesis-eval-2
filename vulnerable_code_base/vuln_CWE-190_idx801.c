void ep2_mul_sim_dig(ep2_t r, const ep2_t p[], const dig_t k[], int len) {
	ep2_t t;
	int max;
	ep2_null(t);
	max = util_bits_dig(k[0]);
	for (int i = 1; i < len; i++) {
		max = RLC_MAX(max, util_bits_dig(k[i]));
	}
	RLC_TRY {
		ep2_new(t);
		ep2_set_infty(t);
		for (int i = max - 1; i >= 0; i--) {
			ep2_dbl(t, t);
			for (int j = 0; j < len; j++) {
				if (k[j] & ((dig_t)1 << i)) {
					ep2_add(t, t, p[j]);
				}
			}
		}
		ep2_norm(r, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep2_free(t);
	}
}