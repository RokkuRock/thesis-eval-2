void fp9_write_bin(uint8_t *bin, int len, const fp9_t a) {
	if (len != 9 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp3_write_bin(bin, 3 * RLC_FP_BYTES, a[0]);
	fp3_write_bin(bin + 3 * RLC_FP_BYTES, 3 * RLC_FP_BYTES, a[1]);
	fp3_write_bin(bin + 6 * RLC_FP_BYTES, 3 * RLC_FP_BYTES, a[2]);
}