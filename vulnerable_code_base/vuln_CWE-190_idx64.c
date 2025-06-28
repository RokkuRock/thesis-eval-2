void ep2_mul_slide(ep2_t r, const ep2_t p, const bn_t k) {
	ep2_t t[1 << (EP_WIDTH - 1)], q;
	int i, j, l;
	uint8_t win[RLC_FP_BITS + 1];
	ep2_null(q);
	if (bn_is_zero(k) || ep2_is_infty(p)) {
		ep2_set_infty(r);
		return;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i ++) {
			ep2_null(t[i]);
			ep2_new(t[i]);
		}
		ep2_new(q);
		ep2_copy(t[0], p);
		ep2_dbl(q, p);
#if defined(EP_MIXED)
		ep2_norm(q, q);
#endif
		for (i = 1; i < (1 << (EP_WIDTH - 1)); i++) {
			ep2_add(t[i], t[i - 1], q);
		}
#if defined(EP_MIXED)
		ep2_norm_sim(t + 1, t + 1, (1 << (EP_WIDTH - 1)) - 1);
#endif
		ep2_set_infty(q);
		l = RLC_FP_BITS + 1;
		bn_rec_slw(win, &l, k, EP_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				ep2_dbl(q, q);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					ep2_dbl(q, q);
				}
				ep2_add(q, q, t[win[i] >> 1]);
			}
		}
		ep2_norm(r, q);
		if (bn_sign(k) == RLC_NEG) {
			ep2_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i++) {
			ep2_free(t[i]);
		}
		ep2_free(q);
	}
}