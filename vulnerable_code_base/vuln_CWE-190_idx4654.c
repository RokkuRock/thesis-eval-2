int rand_check(uint8_t *buf, int size) {
	int count = 0;
	for (int i = 1; i < size; i++) {
		if (buf[i] == buf[i - 1]) {
			count++;
		} else {
			count = 0;
		}
	}
	if (count > RAND_REP) {
		return RLC_ERR;
	}
	return RLC_OK;
}