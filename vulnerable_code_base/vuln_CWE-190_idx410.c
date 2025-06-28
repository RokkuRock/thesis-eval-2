static void eb_mul_rtnaf_imp(eb_t r, const eb_t p, const bn_t k) {
	int i, l, n;
	int8_t tnaf[RLC_FB_BITS + 8], u;
	eb_t t[1 << (EB_WIDTH - 2)];
	if (eb_curve_opt_a() == RLC_ZERO) {
		u = -1;
	} else {
		u = 1;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_null(t[i]);
			eb_new(t[i]);
			eb_set_infty(t[i]);
		}
		l = sizeof(tnaf);
		bn_rec_tnaf(tnaf, &l, k, u, RLC_FB_BITS, EB_WIDTH);
		eb_copy(r, p);
		for (i = 0; i < l; i++) {
			n = tnaf[i];
			if (n > 0) {
				eb_add(t[n / 2], t[n / 2], r);
			}
			if (n < 0) {
				eb_sub(t[-n / 2], t[-n / 2], r);
			}
			fb_sqr(r->x, r->x);
			fb_sqr(r->y, r->y);
		}
		eb_copy(r, t[0]);
#if defined(EB_MIXED) && defined(STRIP) && (EB_WIDTH > 2)
		eb_norm_sim(t + 1, (const eb_t *)t + 1, (1 << (EB_WIDTH - 2)) - 1);
#endif
#if EB_WIDTH == 3
		eb_frb(t[0], t[1]);
		if (u == 1) {
			eb_sub(t[1], t[1], t[0]);
		} else {
			eb_add(t[1], t[1], t[0]);
		}
#endif
#if EB_WIDTH == 4
		eb_frb(t[0], t[3]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == 1) {
			eb_neg(t[0], t[0]);
		}
		eb_sub(t[3], t[0], t[3]);
		eb_frb(t[0], t[1]);
		eb_frb(t[0], t[0]);
		eb_sub(t[1], t[0], t[1]);
		eb_frb(t[0], t[2]);
		eb_frb(t[0], t[0]);
		eb_add(t[2], t[0], t[2]);
#endif
#if EB_WIDTH == 5
		eb_frb(t[0], t[3]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == 1) {
			eb_neg(t[0], t[0]);
		}
		eb_sub(t[3], t[0], t[3]);
		eb_frb(t[0], t[1]);
		eb_frb(t[0], t[0]);
		eb_sub(t[1], t[0], t[1]);
		eb_frb(t[0], t[2]);
		eb_frb(t[0], t[0]);
		eb_add(t[2], t[0], t[2]);
		eb_frb(t[0], t[4]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[4]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == 1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[4], t[0], t[4]);
		eb_frb(t[0], t[5]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[5]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_neg(t[0], t[0]);
		eb_sub(t[5], t[0], t[5]);
		eb_frb(t[0], t[6]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[6]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_neg(t[0], t[0]);
		eb_add(t[6], t[0], t[6]);
		eb_frb(t[0], t[7]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_sub(t[7], t[0], t[7]);
#endif
#if EB_WIDTH == 6
		eb_frb(t[0], t[1]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[0], t[0], t[1]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_sub(t[1], t[0], t[1]);
		eb_frb(t[0], t[2]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[0], t[0], t[2]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_add(t[2], t[0], t[2]);
		eb_frb(t[0], t[3]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[3]);
		eb_neg(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_sub(t[3], t[0], t[3]);
		eb_frb(t[0], t[4]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[4]);
		eb_neg(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[4], t[0], t[4]);
		eb_frb(t[0], t[5]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[5]);
		eb_neg(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_sub(t[5], t[0], t[5]);
		eb_frb(t[0], t[6]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[6]);
		eb_neg(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_add(t[6], t[0], t[6]);
		eb_frb(t[0], t[7]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_sub(t[7], t[0], t[7]);
		eb_frb(t[0], t[8]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_add(t[8], t[0], t[8]);
		eb_frb(t[0], t[9]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[0], t[0], t[9]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_sub(t[0], t[0], t[9]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[9]);
		eb_neg(t[9], t[0]);
		eb_frb(t[0], t[10]);
		eb_frb(t[0], t[0]);
		eb_neg(t[0], t[0]);
		eb_add(t[0], t[0], t[10]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_add(t[10], t[0], t[10]);
		eb_frb(t[0], t[11]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_sub(t[11], t[0], t[11]);
		eb_frb(t[0], t[12]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_add(t[12], t[0], t[12]);
		eb_frb(t[0], t[13]);
		eb_frb(t[0], t[0]);
		eb_add(t[0], t[0], t[13]);
		eb_neg(t[13], t[0]);
		eb_frb(t[0], t[14]);
		eb_frb(t[0], t[0]);
		eb_neg(t[0], t[0]);
		eb_add(t[14], t[0], t[14]);
		eb_frb(t[0], t[15]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		eb_frb(t[0], t[0]);
		if (u == -1) {
			eb_neg(t[0], t[0]);
		}
		eb_sub(t[15], t[0], t[15]);
#endif
#if defined(EB_MIXED) && defined(STRIP) && (EB_WIDTH > 2)
		eb_norm_sim(t + 1, (const eb_t *)t + 1, (1 << (EB_WIDTH - 2)) - 1);
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