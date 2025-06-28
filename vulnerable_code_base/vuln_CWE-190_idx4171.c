static int rand_inc(uint8_t *data, int size, int digit) {
	int carry = digit;
	for (int i = size - 1; i >= 0; i--) {
		int16_t s;
		s = (data[i] + carry);
		data[i] = s & 0xFF;
		carry = s >> 8;
	}
	return carry;
}