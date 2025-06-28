void eb_read_bin(eb_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			eb_set_infty(a);
			return;
		} else {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		}
	}
	if (len != (RLC_FB_BYTES + 1) && len != (2 * RLC_FB_BYTES + 1)) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	a->coord = BASIC;
	fb_set_dig(a->z, 1);
	fb_read_bin(a->x, bin + 1, RLC_FB_BYTES);
	if (len == RLC_FB_BYTES + 1) {
		switch(bin[0]) {
			case 2:
				fb_zero(a->y);
				break;
			case 3:
				fb_zero(a->y);
				fb_set_bit(a->y, 0, 1);
				break;
			default:
				RLC_THROW(ERR_NO_VALID);
				break;
		}
		eb_upk(a, a);
	}
	if (len == 2 * RLC_FB_BYTES + 1) {
		if (bin[0] == 4) {
			fb_read_bin(a->y, bin + RLC_FB_BYTES + 1, RLC_FB_BYTES);
		} else {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
	}
	if (!eb_on_curve(a)) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}