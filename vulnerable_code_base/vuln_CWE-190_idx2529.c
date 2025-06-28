static int rand_add(uint8_t *state, uint8_t *hash, int size) {
	int carry = 0;
	for (int i = size - 1; i >= 0; i--) {
		int16_t s;
		s = (state[i] + hash[i] + carry);
		state[i] = s & 0xFF;
		carry = s >> 8;
	}
	return carry;
}