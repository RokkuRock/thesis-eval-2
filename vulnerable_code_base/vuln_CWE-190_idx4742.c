void fp54_write_bin(uint8_t *bin, int len, const fp54_t a, int pack) {
	fp54_t t;
	fp54_null(t);
	RLC_TRY {
		fp54_new(t);
		if (pack) {
			if (len != 36 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp54_pck(t, a);
			fp9_write_bin(bin, 9 * RLC_FP_BYTES, a[1][0]);
			fp9_write_bin(bin + 9 * RLC_FP_BYTES, 9 * RLC_FP_BYTES, a[1][1]);
			fp9_write_bin(bin + 18 * RLC_FP_BYTES, 9 * RLC_FP_BYTES, a[2][0]);
			fp9_write_bin(bin + 27 * RLC_FP_BYTES, 9 * RLC_FP_BYTES, a[2][1]);
		} else {
			if (len != 54 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp18_write_bin(bin, 18 * RLC_FP_BYTES, a[0]);
			fp18_write_bin(bin + 18 * RLC_FP_BYTES, 18 * RLC_FP_BYTES, a[1]);
			fp18_write_bin(bin + 36 * RLC_FP_BYTES, 18 * RLC_FP_BYTES, a[2]);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp54_free(t);
	}
}