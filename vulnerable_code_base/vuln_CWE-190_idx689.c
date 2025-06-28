void fp2_write_bin(uint8_t *bin, int len, const fp2_t a, int pack) {
	fp2_t t;
	fp2_null(t);
	RLC_TRY {
		fp2_new(t);
		if (pack && fp2_test_cyc(a)) {
			if (len < RLC_FP_BYTES + 1) {
				RLC_THROW(ERR_NO_BUFFER);
				return;
			} else {
				fp2_pck(t, a);
				fp_write_bin(bin, RLC_FP_BYTES, t[0]);
				bin[RLC_FP_BYTES] = fp_get_bit(t[1], 0);
			}
		} else {
			if (len < 2 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
				return;
			} else {
				fp_write_bin(bin, RLC_FP_BYTES, a[0]);
				fp_write_bin(bin + RLC_FP_BYTES, RLC_FP_BYTES, a[1]);
			}
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp2_free(t);
	}
}