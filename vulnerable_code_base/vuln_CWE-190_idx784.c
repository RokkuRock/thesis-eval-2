void bn_rec_win(uint8_t *win, int *len, const bn_t k, int w) {
	int i, j, l;
	l = bn_bits(k);
	if (*len < RLC_CEIL(l, w)) {
		*len = 0;
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	memset(win, 0, *len);
	j = 0;
	for (i = 0; i < l - w; i += w) {
		win[j++] = get_bits(k, i, i + w - 1);
	}
	win[j++] = get_bits(k, i, bn_bits(k) - 1);
	*len = j;
}