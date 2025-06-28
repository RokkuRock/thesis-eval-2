void fp18_read_bin(fp18_t a, const uint8_t *bin, int len) {
	if (len != 18 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp9_read_bin(a[0], bin, 9 * RLC_FP_BYTES);
	fp9_read_bin(a[1], bin + 9 * RLC_FP_BYTES, 9 * RLC_FP_BYTES);
}