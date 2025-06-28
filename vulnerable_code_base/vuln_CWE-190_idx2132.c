void fp2_read_bin(fp2_t a, const uint8_t *bin, int len) {
	if (len != RLC_FP_BYTES + 1 && len != 2 * RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (len == RLC_FP_BYTES + 1) {
		fp_read_bin(a[0], bin, RLC_FP_BYTES);
		fp_zero(a[1]);
		fp_set_bit(a[1], 0, bin[RLC_FP_BYTES]);
		fp2_upk(a, a);
	}
	if (len == 2 * RLC_FP_BYTES) {
		fp_read_bin(a[0], bin, RLC_FP_BYTES);
		fp_read_bin(a[1], bin + RLC_FP_BYTES, RLC_FP_BYTES);
	}
}