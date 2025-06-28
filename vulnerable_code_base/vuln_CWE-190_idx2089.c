static void ed_mul_naf_imp(ed_t r, const ed_t p, const bn_t k) {
	int l, i, n;
	int8_t naf[RLC_FP_BITS + 1];
	ed_t t[1 << (ED_WIDTH - 2)];
	if (bn_is_zero(k)) {
		ed_set_infty(r);
		return;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
			ed_null(t[i]);
			ed_new(t[i]);
		}
		ed_tab(t, p, ED_WIDTH);
		l = sizeof(naf);
		bn_rec_naf(naf, &l, k, EP_WIDTH);
		ed_set_infty(r);
		for (i = l - 1; i > 0; i--) {
			n = naf[i];
			if (n == 0) {
#if ED_ADD == EXTND
				r->coord = EXTND;
#endif
			}
			ed_dbl(r, r);
			if (n > 0) {
				ed_add(r, r, t[n / 2]);
			} else if (n < 0) {
				ed_sub(r, r, t[-n / 2]);
			}
		}
		n = naf[0];
		ed_dbl(r, r);
		if (n > 0) {
			ed_add(r, r, t[n / 2]);
		} else if (n < 0) {
			ed_sub(r, r, t[-n / 2]);
		}
		ed_norm(r, r);
		if (bn_sign(k) == RLC_NEG) {
			ed_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
			ed_free(t[i]);
		}
	}
}