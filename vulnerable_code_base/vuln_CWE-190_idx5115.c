void ed_write_bin(uint8_t *bin, int len, const ed_t a, int pack) {
	ed_t t;
	ed_null(t);
	memset(bin, 0, len);
	if (ed_is_infty(a)) {
		if (len < 1) {
			RLC_THROW(ERR_NO_BUFFER);
			return;
		} else {
			return;
		}
	}
	RLC_TRY {
		ed_new(t);
		ed_norm(t, a);
		if (pack) {
			if (len < RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				ed_pck(t, t);
				bin[0] = 2 | fp_get_bit(t->x, 0);
				fp_write_bin(bin + 1, RLC_FP_BYTES, t->y);
			}
		} else {
			if (len < 2 * RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
			} else {
				bin[0] = 4;
				fp_write_bin(bin + 1, RLC_FP_BYTES, t->y);
				fp_write_bin(bin + RLC_FP_BYTES + 1, RLC_FP_BYTES, t->x);
			}
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		ed_free(t);
	}
}