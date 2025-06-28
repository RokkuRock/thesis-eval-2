void ep_write_bin(uint8_t *bin, int len, const ep_t a, int pack) {
	ep_t t;
	ep_null(t);
	memset(bin, 0, len);
	if (ep_is_infty(a)) {
		if (len < 1) {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		} else {
			return;
		}
	}
	RLC_TRY {
		ep_new(t);
		ep_norm(t, a);
		if (pack) {
			if (len < RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				ep_pck(t, t);
				bin[0] = 2 | fp_get_bit(t->y, 0);
				fp_write_bin(bin + 1, RLC_FP_BYTES, t->x);
			}
		} else {
			if (len < 2 * RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				bin[0] = 4;
				fp_write_bin(bin + 1, RLC_FP_BYTES, t->x);
				fp_write_bin(bin + RLC_FP_BYTES + 1, RLC_FP_BYTES, t->y);
			}
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ep_free(t);
	}
}