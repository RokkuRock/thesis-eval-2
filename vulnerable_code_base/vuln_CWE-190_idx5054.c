void fp54_read_bin(fp54_t a, const uint8_t *bin, int len) {
	if (len != 36 * RLC_FP_BYTES && len != 54 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (len == 36 * RLC_FP_BYTES) {
		fp9_zero(a[0][0]);
		fp9_zero(a[0][1]);
		fp9_read_bin(a[1][0], bin, 9 * RLC_FP_BYTES);
		fp9_read_bin(a[1][1], bin + 9 * RLC_FP_BYTES, 9 * RLC_FP_BYTES);
		fp9_read_bin(a[2][0], bin + 18 * RLC_FP_BYTES, 9 * RLC_FP_BYTES);
		fp9_read_bin(a[2][1], bin + 27 * RLC_FP_BYTES, 9 * RLC_FP_BYTES);
		fp54_back_cyc(a, a);
	}
	if (len == 54 * RLC_FP_BYTES) {
		fp18_read_bin(a[0], bin, 18 * RLC_FP_BYTES);
		fp18_read_bin(a[1], bin + 18 * RLC_FP_BYTES, 18 * RLC_FP_BYTES);
		fp18_read_bin(a[2], bin + 36 * RLC_FP_BYTES, 18 * RLC_FP_BYTES);
	}
}