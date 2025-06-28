void ep4_write_bin(uint8_t *bin, int len, const ep4_t a, int pack) {
	ep4_t t;
	ep4_null(t);
	memset(bin, 0, len);
	if (ep4_is_infty(a)) {
		if (len < 1) {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		} else {
			return;
		}
	}
	RLC_TRY {
		ep4_new(t);
		ep4_norm(t, a);
		if (len < 8 * RLC_FP_BYTES + 1) {
			RLC_THROW(ERR_NO_BUFFER);
		} else {
			bin[0] = 4;
			fp4_write_bin(bin + 1, 4 * RLC_FP_BYTES, t->x);
			fp4_write_bin(bin + 4 * RLC_FP_BYTES + 1, 4 * RLC_FP_BYTES, t->y);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		ep4_free(t);
	}
}