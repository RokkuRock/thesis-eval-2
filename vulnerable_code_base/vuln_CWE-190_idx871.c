void fp9_read_bin(fp9_t a, const uint8_t *bin, int len) {
	if (len != 9 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp3_read_bin(a[0], bin, 3 * RLC_FP_BYTES);
	fp3_read_bin(a[1], bin + 3 * RLC_FP_BYTES, 3 * RLC_FP_BYTES);
	fp3_read_bin(a[2], bin + 6 * RLC_FP_BYTES, 3 * RLC_FP_BYTES);
}