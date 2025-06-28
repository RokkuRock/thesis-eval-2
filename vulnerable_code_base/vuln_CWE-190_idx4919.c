void fp3_write_bin(uint8_t *bin, int len, const fp3_t a) {
	if (len != 3 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	fp_write_bin(bin, RLC_FP_BYTES, a[0]);
	fp_write_bin(bin + RLC_FP_BYTES, RLC_FP_BYTES, a[1]);
	fp_write_bin(bin + 2 * RLC_FP_BYTES, RLC_FP_BYTES, a[2]);
}