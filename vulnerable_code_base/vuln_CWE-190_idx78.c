int fb_size_str(const fb_t a, int radix) {
	bn_t t;
	int digits = 0;
	bn_null(t);
	if (!valid_radix(radix)) {
		RLC_THROW(ERR_NO_VALID);
		return 0;
	}
	RLC_TRY {
		bn_new(t);
		bn_read_raw(t, a, RLC_FB_DIGS);
		digits = bn_size_str(t, radix);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
	return digits;
}