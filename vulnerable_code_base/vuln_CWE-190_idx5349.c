void fp8_write_bin(uint8_t *bin, int len, const fp8_t a) {
	if (len != 8 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp4_write_bin(bin, 4 * RLC_FP_BYTES, a[0]);
	fp4_write_bin(bin + 4 * RLC_FP_BYTES, 4 * RLC_FP_BYTES, a[1]);
}