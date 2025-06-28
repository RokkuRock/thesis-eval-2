void ep4_mul_sim_dig(ep4_t r, const ep4_t p[], const dig_t k[], int len) {
	ep4_t t;
	int max;
	ep4_null(t);
	max = util_bits_dig(k[0]);
	for (int i = 1; i < len; i++) {
		max = RLC_MAX(max, util_bits_dig(k[i]));
	}
	RLC_TRY {
		ep4_new(t);
		ep4_set_infty(t);
		for (int i = max - 1; i >= 0; i--) {
			ep4_dbl(t, t);
			for (int j = 0; j < len; j++) {
				if (k[j] & ((dig_t)1 << i)) {
					ep4_add(t, t, p[j]);
				}
			}
		}
		ep4_norm(r, t);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep4_free(t);
	}
}