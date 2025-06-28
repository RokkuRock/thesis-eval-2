void eb_write_bin(uint8_t *bin, int len, const eb_t a, int pack) {
	eb_t t;
	eb_null(t);
	memset(bin, 0, len);
	if (eb_is_infty(a)) {
		if (len < 1) {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		} else {
			return;
		}
	}
	RLC_TRY {
		eb_new(t);
		eb_norm(t, a);
		if (pack) {
			if (len < RLC_FB_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				eb_pck(t, t);
				bin[0] = 2 | fb_get_bit(t->y, 0);
				fb_write_bin(bin + 1, RLC_FB_BYTES, t->x);
			}
		} else {
			if (len < 2 * RLC_FB_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				bin[0] = 4;
				fb_write_bin(bin + 1, RLC_FB_BYTES, t->x);
				fb_write_bin(bin + RLC_FB_BYTES + 1, RLC_FB_BYTES, t->y);
			}
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		eb_free(t);
	}
}