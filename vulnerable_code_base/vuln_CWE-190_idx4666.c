void ep2_write_bin(uint8_t *bin, int len, const ep2_t a, int pack) {
	ep2_t t;
	ep2_null(t);
	memset(bin, 0, len);
	if (ep2_is_infty(a)) {
		if (len < 1) {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		} else {
			return;
		}
	}
	RLC_TRY {
		ep2_new(t);
		ep2_norm(t, a);
		if (pack) {
			if (len < 2 * RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				ep2_pck(t, t);
				bin[0] = 2 | fp_get_bit(t->y[0], 0);
				fp2_write_bin(bin + 1, 2 * RLC_FP_BYTES, t->x, 0);
			}
		} else {
			if (len < 4 * RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				bin[0] = 4;
				fp2_write_bin(bin + 1, 2 * RLC_FP_BYTES, t->x, 0);
				fp2_write_bin(bin + 2 * RLC_FP_BYTES + 1, 2 * RLC_FP_BYTES, t->y, 0);
			}
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		ep2_free(t);
	}
}