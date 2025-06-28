static int log_radix(int radix) {
	int l = 0;
	while (radix > 0) {
		radix = radix / 2;
		l++;
	}
	return --l;
}