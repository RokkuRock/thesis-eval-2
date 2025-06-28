void fb_exp_basic(fb_t c, const fb_t a, const bn_t b) {
	int i, l;
	fb_t r;
	if (bn_is_zero(b)) {
		fb_set_dig(c, 1);
		return;
	}
	fb_null(r);
	RLC_TRY {
		fb_new(r);
		l = bn_bits(b);
		fb_copy(r, a);
		for (i = l - 2; i >= 0; i--) {
			fb_sqr(r, r);
			if (bn_get_bit(b, i)) {
				fb_mul(r, r, a);
			}
		}
		if (bn_sign(b) == RLC_NEG) {
			fb_inv(c, r);
		} else {
			fb_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fb_free(r);
	}
}