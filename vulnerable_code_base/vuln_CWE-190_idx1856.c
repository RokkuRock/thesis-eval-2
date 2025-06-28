void fp4_read_bin(fp4_t a, const uint8_t *bin, int len) {
	if (len != 4 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp2_read_bin(a[0], bin, 2 * RLC_FP_BYTES);
	fp2_read_bin(a[1], bin + 2 * RLC_FP_BYTES, 2 * RLC_FP_BYTES);
}