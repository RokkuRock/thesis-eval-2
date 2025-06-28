void fp6_write_bin(uint8_t *bin, int len, const fp6_t a) {
	if (len != 6 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp2_write_bin(bin, 2 * RLC_FP_BYTES, a[0], 0);
	fp2_write_bin(bin + 2 * RLC_FP_BYTES, 2 * RLC_FP_BYTES, a[1], 0);
	fp2_write_bin(bin + 4 * RLC_FP_BYTES, 2 * RLC_FP_BYTES, a[2], 0);
}