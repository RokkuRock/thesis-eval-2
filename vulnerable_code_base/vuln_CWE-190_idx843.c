void fb_read_bin(fb_t a, const uint8_t *bin, int len) {
	bn_t t;
	bn_null(t);
	if (len != RLC_FB_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(t);
		bn_read_bin(t, bin, len);
		fb_copy(a, t->dp);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}