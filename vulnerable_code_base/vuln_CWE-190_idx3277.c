void ep4_read_bin(ep4_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			ep4_set_infty(a);
			return;
		} else {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		}
	}
	if (len != (8 * RLC_FP_BYTES + 1)) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	a->coord = BASIC;
	fp4_set_dig(a->z, 1);
	fp4_read_bin(a->x, bin + 1, 4 * RLC_FP_BYTES);
	if (len == 8 * RLC_FP_BYTES + 1) {
		if (bin[0] == 4) {
			fp4_read_bin(a->y, bin + 4 * RLC_FP_BYTES + 1, 4 * RLC_FP_BYTES);
		} else {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
	}
	if (!ep4_on_curve(a)) {
		RLC_THROW(ERR_NO_VALID);
	}
}