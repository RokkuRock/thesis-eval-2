void fp12_read_bin(fp12_t a, const uint8_t *bin, int len) {
	if (len != 8 * RLC_FP_BYTES && len != 12 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (len == 8 * RLC_FP_BYTES) {
		fp2_zero(a[0][0]);
		fp2_read_bin(a[0][1], bin, 2 * RLC_FP_BYTES);
		fp2_read_bin(a[0][2], bin + 2 * RLC_FP_BYTES, 2 * RLC_FP_BYTES);
		fp2_read_bin(a[1][0], bin + 4 * RLC_FP_BYTES, 2 * RLC_FP_BYTES);
		fp2_zero(a[1][1]);
		fp2_read_bin(a[1][2], bin + 6 * RLC_FP_BYTES, 2 * RLC_FP_BYTES);
		fp12_back_cyc(a, a);
	}
	if (len == 12 * RLC_FP_BYTES) {
		fp6_read_bin(a[0], bin, 6 * RLC_FP_BYTES);
		fp6_read_bin(a[1], bin + 6 * RLC_FP_BYTES, 6 * RLC_FP_BYTES);
	}
}