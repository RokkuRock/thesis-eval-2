dig_t bn_get_prime(int pos) {
	if (pos >= BASIC_TESTS) {
		return 0;
	}
	return primes[pos];
}