void ep2_mul_basic(ep2_t r, const ep2_t p, const bn_t k) {
	int i, l;
	ep2_t t;
	ep2_null(t);
	if (bn_is_zero(k) || ep2_is_infty(p)) {
		ep2_set_infty(r);
		return;
	}
	RLC_TRY {
		ep2_new(t);
		l = bn_bits(k);
		if (bn_get_bit(k, l - 1)) {
			ep2_copy(t, p);
		} else {
			ep2_set_infty(t);
		}
		for (i = l - 2; i >= 0; i--) {
			ep2_dbl(t, t);
			if (bn_get_bit(k, i)) {
				ep2_add(t, t, p);
			}
		}
		ep2_copy(r, t);
		ep2_norm(r, r);
		if (bn_sign(k) == RLC_NEG) {
			ep2_neg(r, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep2_free(t);
	}
}