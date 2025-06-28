void fp12_write_bin(uint8_t *bin, int len, const fp12_t a, int pack) {
	fp12_t t;
	fp12_null(t);
	RLC_TRY {
		fp12_new(t);
		if (pack) {
			if (len != 8 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp12_pck(t, a);
			fp2_write_bin(bin, 2 * RLC_FP_BYTES, a[0][1], 0);
			fp2_write_bin(bin + 2 * RLC_FP_BYTES, 2 * RLC_FP_BYTES, a[0][2], 0);
			fp2_write_bin(bin + 4 * RLC_FP_BYTES, 2 * RLC_FP_BYTES, a[1][0], 0);
			fp2_write_bin(bin + 6 * RLC_FP_BYTES, 2 * RLC_FP_BYTES, a[1][2], 0);
		} else {
			if (len != 12 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp6_write_bin(bin, 6 * RLC_FP_BYTES, a[0]);
			fp6_write_bin(bin + 6 * RLC_FP_BYTES, 6 * RLC_FP_BYTES, a[1]);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp12_free(t);
	}
}