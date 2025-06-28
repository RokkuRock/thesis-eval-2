void ed_read_bin(ed_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			ed_set_infty(a);
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
	fp_read_bin(a->y, bin + 1, RLC_FP_BYTES);
	if (len == RLC_FP_BYTES + 1) {
		switch (bin[0]) {
			case 2:
				fp_zero(a->x);
				break;
			case 3:
				fp_zero(a->x);
				fp_set_bit(a->x, 0, 1);
				break;
			default:
				RLC_THROW(ERR_NO_VALID);
				break;
		}
		ed_upk(a, a);
	}
	if (len == 2 * RLC_FP_BYTES + 1) {
		if (bin[0] == 4) {
			fp_read_bin(a->x, bin + RLC_FP_BYTES + 1, RLC_FP_BYTES);
		} else {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
	}
#if ED_ADD == EXTND
	fp_mul(a->t, a->x, a->y);
	fp_mul(a->x, a->x, a->z);
	fp_mul(a->y, a->y, a->z);
	fp_sqr(a->z, a->z);
#endif
	if (!ed_on_curve(a)) {
		RLC_THROW(ERR_NO_VALID);
	}
}