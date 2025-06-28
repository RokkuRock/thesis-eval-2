void ed_mul_slide(ed_t r, const ed_t p, const bn_t k) {
	ed_t t[1 << (EP_WIDTH - 1)], q;
	int i, j, l;
	uint8_t win[RLC_FP_BITS + 1];
	ed_null(q);
	if (bn_is_zero(k) || ed_is_infty(p)) {
		ed_set_infty(r);
		return;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i ++) {
			ed_null(t[i]);
			ed_new(t[i]);
		}
		ed_new(q);
		ed_copy(t[0], p);
		ed_dbl(q, p);
#if defined(EP_MIXED)
		ed_norm(q, q);
#endif
		for (i = 1; i < (1 << (EP_WIDTH - 1)); i++) {
			ed_add(t[i], t[i - 1], q);
		}
#if defined(EP_MIXED)
		ed_norm_sim(t + 1, (const ed_t *)t + 1, (1 << (EP_WIDTH - 1)) - 1);
#endif
		ed_set_infty(q);
		l = RLC_FP_BITS + 1;
		bn_rec_slw(win, &l, k, EP_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				ed_dbl(q, q);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					ed_dbl(q, q);
				}
				ed_add(q, q, t[win[i] >> 1]);
			}
		}
		ed_norm(r, q);
		if (bn_sign(k) == RLC_NEG) {
			ed_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i++) {
			ed_free(t[i]);
		}
		ed_free(q);
	}
}