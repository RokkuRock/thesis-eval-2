void bn_rec_tnaf_mod(bn_t r0, bn_t r1, const bn_t k, int u, int m) {
	bn_t t, t0, t1, t2, t3;
	bn_null(t);
	bn_null(t0);
	bn_null(t1);
	bn_null(t2);
	bn_null(t3);
	RLC_TRY {
		bn_new(t);
		bn_new(t0);
		bn_new(t1);
		bn_new(t2);
		bn_new(t3);
		bn_set_dig(t0, 1);
		bn_zero(t1);
		bn_zero(t2);
		bn_zero(t3);
		bn_abs(r0, k);
		bn_zero(r1);
		for (int i = 0; i < m; i++) {
			if (!bn_is_even(r0)) {
				bn_sub_dig(r0, r0, 1);
				bn_add(t2, t2, t0);
				bn_add(t3, t3, t1);
			}
			bn_hlv(t, r0);
			if (u == -1) {
				bn_sub(r0, r1, t);
			} else {
				bn_add(r0, r1, t);
			}
			bn_neg(r1, t);
			bn_dbl(t, t1);
			if (u == -1) {
				bn_sub(t1, t0, t1);
			} else {
				bn_add(t1, t0, t1);
			}
			bn_neg(t0, t);
		}
		bn_add(r0, r0, t2);
		bn_add(r1, r1, t3);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
		bn_free(t0);
		bn_free(t1);
		bn_free(t2);
		bn_free(t3);
	}
}