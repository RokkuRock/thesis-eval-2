static int square_root(void) {
	int bits, code = RLC_ERR;
	bn_t a, b, c;
	bn_null(a);
	bn_null(b);
	bn_null(c);
	RLC_TRY {
		bn_new(a);
		bn_new(b);
		bn_new(c);
		TEST_ONCE("square root extraction is correct") {
			for (bits = 0; bits < RLC_BN_BITS / 2; bits++) {
				bn_rand(a, RLC_POS, bits);
				bn_sqr(c, a);
				bn_srt(b, c);
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
			for (bits = 0; bits < RLC_BN_BITS; bits++) {
				bn_rand(a, RLC_POS, bits);
				bn_srt(b, a);
				bn_sqr(c, b);
				TEST_ASSERT(bn_cmp(c, a) != RLC_GT, end);
			}
		}
		TEST_END;
		TEST_ONCE("square root of powers of 2 is correct") {
			for (bits = 0; bits < RLC_BN_BITS / 2; bits++) {
				bn_set_2b(a, bits);
				bn_sqr(c, a);
				bn_srt(b, c);
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	bn_free(a);
	bn_free(b);
	bn_free(c);
	return code;
}