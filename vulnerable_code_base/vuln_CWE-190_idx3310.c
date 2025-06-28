void fb_read_str(fb_t a, const char *str, int len, int radix) {
	bn_t t;
	bn_null(t);
	if (!valid_radix(radix)) {
		RLC_THROW(ERR_NO_VALID);
	}
	RLC_TRY {
		bn_new(t);
		bn_read_str(t, str, len, radix);
		if (bn_bits(t) > RLC_FB_BITS) {
			RLC_THROW(ERR_NO_BUFFER);
		}
		fb_zero(a);
		dv_copy(a, t->dp, t->used);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}