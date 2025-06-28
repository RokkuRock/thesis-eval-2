void fp8_read_bin(fp8_t a, const uint8_t *bin, int len) {
	if (len != 8 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp4_read_bin(a[0], bin, 4 * RLC_FP_BYTES);
	fp4_read_bin(a[1], bin + 4 * RLC_FP_BYTES, 4 * RLC_FP_BYTES);
}