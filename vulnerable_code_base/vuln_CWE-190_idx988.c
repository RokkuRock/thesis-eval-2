void bn_rec_slw(uint8_t *win, int *len, const bn_t k, int w) {
	int i, j, l, s;
	l = bn_bits(k);
	if (*len < l) {
		*len = 0;
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	memset(win, 0, *len);
	i = l - 1;
	j = 0;
	while (i >= 0) {
		if (!bn_get_bit(k, i)) {
			i--;
			win[j++] = 0;
		} else {
			s = RLC_MAX(i - w + 1, 0);
			while (!bn_get_bit(k, s)) {
				s++;
			}
			win[j++] = get_bits(k, s, i);
			i = s - 1;
		}
	}
	*len = j;
}