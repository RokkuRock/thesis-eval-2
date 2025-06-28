static void eb_mul_ltnaf_imp(eb_t r, const eb_t p, const bn_t k) {
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
		}
		eb_tab(t, p, EB_WIDTH);
		l = sizeof(tnaf);
		bn_rec_tnaf(tnaf, &l, k, u, RLC_FB_BITS, EB_WIDTH);
		n = tnaf[l - 1];
		if (n > 0) {
			eb_copy(r, t[n / 2]);
		} else {
			eb_neg(r, t[-n / 2]);
		}
		for (i = l - 2; i >= 0; i--) {
			eb_frb(r, r);
			n = tnaf[i];
			if (n > 0) {
				eb_add(r, r, t[n / 2]);
			}
			if (n < 0) {
				eb_sub(r, r, t[-n / 2]);
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