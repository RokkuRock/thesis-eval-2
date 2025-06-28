void fb_write_bin(uint8_t *bin, int len, const fb_t a) {
	bn_t t;
	bn_null(t);
	if (len != RLC_FB_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(t);
		bn_read_raw(t, a, RLC_FB_DIGS);
		bn_write_bin(bin, len, t);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}