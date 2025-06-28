void fp48_read_bin(fp48_t a, const uint8_t *bin, int len) {
	if (len != 32 * RLC_FP_BYTES && len != 48 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (len == 32 * RLC_FP_BYTES) {
		fp8_zero(a[0][0]);
		fp8_read_bin(a[0][1], bin, 8 * RLC_FP_BYTES);
		fp8_read_bin(a[0][2], bin + 8 * RLC_FP_BYTES, 8 * RLC_FP_BYTES);
		fp8_read_bin(a[1][0], bin + 16 * RLC_FP_BYTES, 8 * RLC_FP_BYTES);
		fp8_zero(a[1][1]);
		fp8_read_bin(a[1][2], bin + 24 * RLC_FP_BYTES, 8 * RLC_FP_BYTES);
		fp48_back_cyc(a, a);
	}
	if (len == 48 * RLC_FP_BYTES) {
		fp24_read_bin(a[0], bin, 24 * RLC_FP_BYTES);
		fp24_read_bin(a[1], bin + 24 * RLC_FP_BYTES, 24 * RLC_FP_BYTES);
	}
}