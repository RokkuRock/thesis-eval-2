static int test(void) {
	uint8_t out[64];
	int len = sizeof(out) / 2, code = RLC_ERR;
	TEST_ONCE("rdrand hardware generator is non-trivial") {
		memset(out, 0, 2 * len);
		rand_bytes(out, len);
		TEST_ASSERT(memcmp(out, out + len, len) != 0, end);
	}
	TEST_END;
	code = RLC_OK;
  end:
	return code;
}