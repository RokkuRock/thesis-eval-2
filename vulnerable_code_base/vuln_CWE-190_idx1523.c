void ep_read_bin(ep_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			ep_set_infty(a);
			return;
		} else {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		}
	}
	if (len != (RLC_FP_BYTES + 1) && len != (2 * RLC_FP_BYTES + 1)) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	a->coord = BASIC;
	fp_set_dig(a->z, 1);
	fp_read_bin(a->x, bin + 1, RLC_FP_BYTES);
	if (len == RLC_FP_BYTES + 1) {
		switch(bin[0]) {
			case 2:
				fp_zero(a->y);
				break;
			case 3:
				fp_zero(a->y);
				fp_set_bit(a->y, 0, 1);
				break;
			default:
				RLC_THROW(ERR_NO_VALID);
				break;
		}
		ep_upk(a, a);
	}
	if (len == 2 * RLC_FP_BYTES + 1) {
		if (bin[0] == 4) {
			fp_read_bin(a->y, bin + RLC_FP_BYTES + 1, RLC_FP_BYTES);
		} else {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
	}
	if (!ep_on_curve(a)) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}