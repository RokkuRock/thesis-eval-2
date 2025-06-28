int fp_bits(const fp_t a) {
	int i = RLC_FP_DIGS - 1;
	while (i >= 0 && a[i] == 0) {
		i--;
	}
	if (i > 0) {
		return (i << RLC_DIG_LOG) + util_bits_dig(a[i]);
	} else {
		return util_bits_dig(a[0]);
	}
}