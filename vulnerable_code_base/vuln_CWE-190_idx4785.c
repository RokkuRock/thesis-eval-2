static void eb_mul_lnaf_imp(eb_t r, const eb_t p, const bn_t k) {
	int i, l, n;
	int8_t naf[RLC_FB_BITS + 1];
	eb_t t[1 << (EB_WIDTH - 2)];
	RLC_TRY {
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_null(t[i]);
			eb_new(t[i]);
			eb_set_infty(t[i]);
			fb_set_dig(t[i]->z, 1);
			t[i]->coord = BASIC;
		}
		eb_tab(t, p, EB_WIDTH);
		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, EB_WIDTH);
		n = naf[l - 1];
		if (n > 0) {
			eb_copy(r, t[n / 2]);
		}
		for (i = l - 2; i >= 0; i--) {
			eb_dbl(r, r);
			n = naf[i];
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