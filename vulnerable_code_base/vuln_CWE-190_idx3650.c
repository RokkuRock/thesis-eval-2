void fb_rand(fb_t a) {
	int bits, digits;
	rand_bytes((uint8_t *)a, RLC_FB_DIGS * sizeof(dig_t));
	RLC_RIP(bits, digits, RLC_FB_BITS);
	if (bits > 0) {
		dig_t mask = RLC_MASK(bits);
		a[RLC_FB_DIGS - 1] &= mask;
	}
}