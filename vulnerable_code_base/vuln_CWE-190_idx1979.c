void fp24_read_bin(fp24_t a, const uint8_t *bin, int len) {
	if (len != 16 * RLC_FP_BYTES && len != 24 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (len == 16 * RLC_FP_BYTES) {
		fp4_zero(a[0][0]);
		fp4_zero(a[0][1]);
		fp4_read_bin(a[1][0], bin, 4 * RLC_FP_BYTES);
		fp4_read_bin(a[1][1], bin + 4 * RLC_FP_BYTES, 4 * RLC_FP_BYTES);
		fp4_read_bin(a[2][0], bin + 8 * RLC_FP_BYTES, 4 * RLC_FP_BYTES);
		fp4_read_bin(a[2][1], bin + 12 * RLC_FP_BYTES, 4 * RLC_FP_BYTES);
		fp24_back_cyc(a, a);
	}
	if (len == 24 * RLC_FP_BYTES) {
		fp8_read_bin(a[0], bin, 8 * RLC_FP_BYTES);
		fp8_read_bin(a[1], bin + 8 * RLC_FP_BYTES, 8 * RLC_FP_BYTES);
		fp8_read_bin(a[2], bin + 16 * RLC_FP_BYTES, 8 * RLC_FP_BYTES);
	}
}