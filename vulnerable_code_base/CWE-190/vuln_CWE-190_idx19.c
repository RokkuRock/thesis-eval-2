void fp48_write_bin(uint8_t *bin, int len, const fp48_t a, int pack) {
	fp48_t t;
	fp48_null(t);
	RLC_TRY {
		fp48_new(t);
		if (pack) {
			if (len != 32 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp48_pck(t, a);
			fp8_write_bin(bin, 8 * RLC_FP_BYTES, a[0][1]);
			fp8_write_bin(bin + 8 * RLC_FP_BYTES, 8 * RLC_FP_BYTES, a[0][2]);
			fp8_write_bin(bin + 16 * RLC_FP_BYTES, 8 * RLC_FP_BYTES, a[1][0]);
			fp8_write_bin(bin + 24 * RLC_FP_BYTES, 8 * RLC_FP_BYTES, a[1][2]);
		} else {
			if (len != 48 * RLC_FP_BYTES) {
				RLC_THROW(ERR_NO_BUFFER);
			}
			fp24_write_bin(bin, 24 * RLC_FP_BYTES, a[0], 0);
			fp24_write_bin(bin + 24 * RLC_FP_BYTES, 24 * RLC_FP_BYTES, a[1], 0);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		fp48_free(t);
	}
}