void ep_mul_slide(ep_t r, const ep_t p, const bn_t k) {
	bn_t _k, n;
	ep_t t[1 << (EP_WIDTH - 1)], q;
	int i, j, l;
	uint8_t win[RLC_FP_BITS + 1];
	if (bn_is_zero(k) || ep_is_infty(p)) {
		ep_set_infty(r);
		return;
	}
	ep_null(q);
	bn_null(n);
	bn_null(_k);
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i ++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		ep_new(q);
		ep_copy(t[0], p);
		ep_dbl(q, p);
#if defined(EP_MIXED)
		ep_norm(q, q);
#endif
		ep_curve_get_ord(n);
		bn_mod(_k, k, n);
		for (i = 1; i < (1 << (EP_WIDTH - 1)); i++) {
			ep_add(t[i], t[i - 1], q);
		}
#if defined(EP_MIXED)
		ep_norm_sim(t + 1, (const ep_t *)t + 1, (1 << (EP_WIDTH - 1)) - 1);
#endif
		ep_set_infty(q);
		l = RLC_FP_BITS + 1;
		bn_rec_slw(win, &l, _k, EP_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				ep_dbl(q, q);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					ep_dbl(q, q);
				}
				ep_add(q, q, t[win[i] >> 1]);
			}
		}
		ep_norm(r, q);
		if (bn_sign(_k) == RLC_NEG) {
			ep_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		for (i = 0; i < (1 << (EP_WIDTH - 1)); i++) {
			ep_free(t[i]);
		}
		ep_free(q);
	}
}