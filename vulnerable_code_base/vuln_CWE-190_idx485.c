void fp24_write_bin(uint8_t *bin, int len, const fp24_t a, int pack) {
	fp24_t t;
	fp24_null(t);
	RLC_TRY {
		fp24_new(t);
		if (pack) {
			if (len != 16 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp24_pck(t, a);
			fp4_write_bin(bin, 4 * RLC_FP_BYTES, a[1][0]);
			fp4_write_bin(bin + 4 * RLC_FP_BYTES, 4 * RLC_FP_BYTES, a[1][1]);
			fp4_write_bin(bin + 8 * RLC_FP_BYTES, 4 * RLC_FP_BYTES, a[2][0]);
			fp4_write_bin(bin + 12 * RLC_FP_BYTES, 4 * RLC_FP_BYTES, a[2][1]);
		} else {
			if (len != 24 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp8_write_bin(bin, 8 * RLC_FP_BYTES, a[0]);
			fp8_write_bin(bin + 8 * RLC_FP_BYTES, 8 * RLC_FP_BYTES, a[1]);
			fp8_write_bin(bin + 16 * RLC_FP_BYTES, 8 * RLC_FP_BYTES, a[2]);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp24_free(t);
	}
}