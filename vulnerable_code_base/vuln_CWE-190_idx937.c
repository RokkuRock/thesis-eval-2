static int valid_radix(int radix) {
	while (radix > 0) {
		if (radix != 1 && radix % 2 == 1)
			return 0;
		radix = radix / 2;
	}
	return 1;
}