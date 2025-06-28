static void eb_mul_rnaf_imp(eb_t r, const eb_t p, const bn_t k) {
	int i, l, n;
	int8_t naf[RLC_FB_BITS + 1];
	eb_t t[1 << (EB_WIDTH - 2)];
	RLC_TRY {
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_null(t[i]);
			eb_new(t[i]);
			eb_set_infty(t[i]);
		}
		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, EB_WIDTH);
		eb_copy(r, p);
		for (i = 0; i < l; i++) {
			n = naf[i];
			if (n > 0) {
				eb_add(t[n / 2], t[n / 2], r);
			}
			if (n < 0) {
				eb_sub(t[-n / 2], t[-n / 2], r);
			}
			eb_dbl(r, r);
		}
		eb_copy(r, t[0]);
#if EB_WIDTH >= 3
		eb_dbl(t[0], t[1]);
		eb_add(t[1], t[0], t[1]);
#endif
#if EB_WIDTH >= 4
		eb_dbl(t[0], t[2]);
		eb_dbl(t[0], t[0]);
		eb_add(t[2], t[0], t[2]);
		eb_dbl(t[0], t[3]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_sub(t[3], t[0], t[3]);
#endif
#if EB_WIDTH >= 5
		eb_dbl(t[0], t[4]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_add(t[4], t[0], t[4]);
		eb_dbl(t[0], t[5]);
		eb_dbl(t[0], t[0]);
		eb_add(t[0], t[0], t[5]);
		eb_dbl(t[0], t[0]);
		eb_add(t[5], t[0], t[5]);
		eb_dbl(t[0], t[6]);
		eb_add(t[0], t[0], t[6]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_add(t[6], t[0], t[6]);
		eb_dbl(t[0], t[7]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_sub(t[7], t[0], t[7]);
#endif
#if EB_WIDTH == 6
		for (i = 8; i < 15; i++) {
			eb_mul_dig(t[i], t[i], 2 * i + 1);
		}
		eb_dbl(t[0], t[15]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_dbl(t[0], t[0]);
		eb_sub(t[15], t[0], t[15]);
#endif
		for (i = 1; i < (1 << (EB_WIDTH - 2)); i++) {
			if (r->coord == BASIC) {
				eb_add(r, t[i], r);
			} else {
				eb_add(r, r, t[i]);
			}
		}
		eb_norm(r, r);
		if (bn_sign(k) == RLC_NEG) {
			eb_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_free(t[i]);
		}
	}
}