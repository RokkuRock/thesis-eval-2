int fb_get_bit(const fb_t a, int bit) {
	int d;
	RLC_RIP(bit, d, bit);
	return (a[d] >> bit) & 1;
}