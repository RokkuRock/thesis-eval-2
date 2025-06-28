void ep4_mul_slide(ep4_t r, const ep4_t p, const bn_t k) {
	ep4_t t[1 << (EP_WIDTH - 1)], q;
	int i, j, l;
	uint8_t win[RLC_FP_BITS + 1];
	ep4_null(q);
	if (bn_is_zero(k) || ep4_is_infty(p)) {
		ep4_set_infty(r);
		return;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i ++) {
			ep4_null(t[i]);
			ep4_new(t[i]);
		}
		ep4_new(q);
		ep4_copy(t[0], p);
		ep4_dbl(q, p);
#if defined(EP_MIXED)
		ep4_norm(q, q);
#endif
		for (i = 1; i < (1 << (EP_WIDTH - 1)); i++) {
			ep4_add(t[i], t[i - 1], q);
		}
#if defined(EP_MIXED)
		ep4_norm_sim(t + 1, t + 1, (1 << (EP_WIDTH - 1)) - 1);
#endif
		ep4_set_infty(q);
		l = RLC_FP_BITS + 1;
		bn_rec_slw(win, &l, k, EP_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				ep4_dbl(q, q);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					ep4_dbl(q, q);
				}
				ep4_add(q, q, t[win[i] >> 1]);
			}
		}
		ep4_norm(r, q);
		if (bn_sign(k) == RLC_NEG) {
			ep4_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i++) {
			ep4_free(t[i]);
		}
		ep4_free(q);
	}
}