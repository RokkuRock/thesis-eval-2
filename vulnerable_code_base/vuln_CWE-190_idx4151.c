int bn_ham(const bn_t a) {
	int c = 0;
	for (int i = 0; i < bn_bits(a); i++) {
		c += bn_get_bit(a, i);
	}
	return c;
}