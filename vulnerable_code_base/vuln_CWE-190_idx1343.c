void fp_set_bit(fp_t a, int bit, int value) {
	int d;
	dig_t mask;
	RLC_RIP(bit, d, bit);
	mask = (dig_t)1 << bit;
	if (value == 1) {
		a[d] |= mask;
	} else {
		a[d] &= ~mask;
	}
}