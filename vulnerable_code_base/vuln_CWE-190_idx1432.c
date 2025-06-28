void ep2_read_bin(ep2_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			ep2_set_infty(a);
			return;
		} else {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		}
	}
	if (len != (2 * RLC_FP_BYTES + 1) && len != (4 * RLC_FP_BYTES + 1)) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	a->coord = BASIC;
	fp2_set_dig(a->z, 1);
	fp2_read_bin(a->x, bin + 1, 2 * RLC_FP_BYTES);
	if (len == 2 * RLC_FP_BYTES + 1) {
		switch(bin[0]) {
			case 2:
				fp2_zero(a->y);
				break;
			case 3:
				fp2_zero(a->y);
				fp_set_bit(a->y[0], 0, 1);
				fp_zero(a->y[1]);
				break;
			default:
				RLC_THROW(ERR_NO_VALID);
				break;
		}
		ep2_upk(a, a);
	}
	if (len == 4 * RLC_FP_BYTES + 1) {
		if (bin[0] == 4) {
			fp2_read_bin(a->y, bin + 2 * RLC_FP_BYTES + 1, 2 * RLC_FP_BYTES);
		} else {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
	}
	if (!ep2_on_curve(a)) {
		RLC_THROW(ERR_NO_VALID);
	}
}