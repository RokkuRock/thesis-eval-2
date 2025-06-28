void fp18_write_bin(uint8_t *bin, int len, const fp18_t a) {
	if (len != 18 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp9_write_bin(bin, 9 * RLC_FP_BYTES, a[0]);
	fp9_write_bin(bin + 9 * RLC_FP_BYTES, 9 * RLC_FP_BYTES, a[1]);
}