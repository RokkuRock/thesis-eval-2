static int util(void) {
	int bits, code = RLC_ERR;
	char str[RLC_BN_BITS + 2];
	dig_t digit, raw[RLC_BN_DIGS];
	uint8_t bin[RLC_CEIL(RLC_BN_BITS, 8)];
	bn_t a, b, c;
	bn_null(a);
	bn_null(b);
	bn_null(c);
	RLC_TRY {
		bn_new(a);
		bn_new(b);
		bn_new(c);
		TEST_CASE("comparison is consistent") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_POS, RLC_BN_BITS);
			if (bn_cmp(a, b) != RLC_EQ) {
				if (bn_cmp(a, b) == RLC_GT) {
					TEST_ASSERT(bn_cmp(b, a) == RLC_LT, end);
				} else {
					TEST_ASSERT(bn_cmp(b, a) == RLC_GT, end);
				}
			}
		}
		TEST_END;
		TEST_CASE("copy and comparison are consistent") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_POS, RLC_BN_BITS);
			bn_rand(c, RLC_POS, RLC_BN_BITS);
			if (bn_cmp(a, c) != RLC_EQ) {
				bn_copy(c, a);
				TEST_ASSERT(bn_cmp(c, a) == RLC_EQ, end);
			}
			if (bn_cmp(b, c) != RLC_EQ) {
				bn_copy(c, b);
				TEST_ASSERT(bn_cmp(b, c) == RLC_EQ, end);
			}
		}
		TEST_END;
		TEST_CASE("absolute, negation and comparison are consistent") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_neg(b, a);
			bn_abs(a, b);
			TEST_ASSERT(bn_cmp(a, b) == RLC_GT, end);
			TEST_ASSERT(bn_cmp(b, a) == RLC_LT, end);
			TEST_ASSERT(bn_cmp_abs(a, b) == RLC_EQ, end);
			TEST_ASSERT(bn_cmp_dig(a, (dig_t)0) == RLC_GT, end);
			TEST_ASSERT(bn_cmp_dig(b, (dig_t)0) == RLC_LT, end);
		} TEST_END;
		TEST_CASE("signal test is correct") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_NEG, RLC_BN_BITS);
			TEST_ASSERT(bn_sign(a) == RLC_POS, end);
			TEST_ASSERT(bn_sign(b) == RLC_NEG, end);
		} TEST_END;
		TEST_CASE("assignment to zero and comparison are consistent") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_NEG, RLC_BN_BITS);
			bn_zero(c);
			TEST_ASSERT(bn_cmp(a, c) == RLC_GT, end);
			TEST_ASSERT(bn_cmp(c, a) == RLC_LT, end);
			TEST_ASSERT(bn_cmp(b, c) == RLC_LT, end);
			TEST_ASSERT(bn_cmp(c, b) == RLC_GT, end);
			TEST_ASSERT(bn_cmp_dig(a, (dig_t)0) == RLC_GT, end);
			TEST_ASSERT(bn_cmp_dig(b, (dig_t)0) == RLC_LT, end);
			TEST_ASSERT(bn_cmp_dig(c, (dig_t)0) == RLC_EQ, end);
		} TEST_END;
		TEST_CASE("assignment to zero and zero test are consistent") {
			bn_zero(c);
			TEST_ASSERT(bn_is_zero(c), end);
			TEST_ASSERT(bn_cmp_dig(c, (dig_t)0) == RLC_EQ, end);
		} TEST_END;
		TEST_CASE("oddness test is correct") {
			bn_set_dig(a, 2);
			bn_set_dig(b, 1);
			TEST_ASSERT(bn_is_even(a) == 1, end);
			TEST_ASSERT(bn_is_even(b) == 0, end);
		} TEST_END;
		bits = 0;
		TEST_CASE("assignment and bit counting are consistent") {
			bn_set_2b(a, bits);
			TEST_ASSERT(bits + 1 == bn_bits(a), end);
			bits = (bits + 1) % RLC_BN_BITS;
		} TEST_END;
		bits = 0;
		TEST_CASE("bit setting and getting are consistent") {
			bn_zero(a);
			bn_set_bit(a, bits, 1);
			TEST_ASSERT(bn_get_bit(a, bits) == 1, end);
			bn_set_bit(a, bits, 0);
			TEST_ASSERT(bn_get_bit(a, bits) == 0, end);
			bits = (bits + 1) % RLC_BN_BITS;
		}
		TEST_END;
		bits = 0;
		TEST_CASE("hamming weight is correct") {
			bn_zero(a);
			for (int j = 0; j < bits; j++) {
				bn_set_bit(a, j, 1);
			}
			TEST_ASSERT(bn_ham(a) == bits, end);
			bits = (bits + 1) % RLC_BN_BITS;
		}
		TEST_END;
		TEST_CASE("generating a random integer is consistent") {
			do {
				bn_rand(b, RLC_POS, RLC_BN_BITS);
			} while (bn_is_zero(b));
			bn_rand_mod(a, b);
			TEST_ASSERT(bn_sign(a) == bn_sign(b), end);
			TEST_ASSERT(bn_is_zero(a) == 0, end);
			TEST_ASSERT(bn_cmp(a, b) == RLC_LT, end);
			do {
				bn_rand(b, RLC_NEG, RLC_DIG);
			} while (bn_bits(b) <= 1);
			bn_rand_mod(a, b);
			TEST_ASSERT(bn_sign(a) == bn_sign(b), end);
			TEST_ASSERT(bn_is_zero(a) == 0, end);
			TEST_ASSERT(bn_cmp(a, b) == RLC_GT, end);
		}
		TEST_END;
		TEST_CASE("reading and writing the first digit are consistent") {
			bn_rand(a, RLC_POS, RLC_DIG);
			bn_rand(b, RLC_POS, RLC_DIG);
			bn_get_dig(&digit, a);
			bn_set_dig(b, digit);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
		} TEST_END;
		TEST_CASE("assignment to a constant and comparison are consistent") {
			bn_set_dig(a, 2);
			bn_set_dig(b, 1);
			TEST_ASSERT(bn_cmp(a, b) == RLC_GT, end);
			TEST_ASSERT(bn_cmp(b, a) == RLC_LT, end);
			TEST_ASSERT(bn_cmp_dig(a, (dig_t)0) == RLC_GT, end);
			TEST_ASSERT(bn_cmp_dig(b, (dig_t)0) == RLC_GT, end);
		} TEST_END;
		TEST_CASE("assignment to random and comparison are consistent") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_NEG, RLC_BN_BITS);
			bn_zero(c);
			TEST_ASSERT(bn_cmp(a, c) == RLC_GT, end);
			TEST_ASSERT(bn_cmp(b, c) == RLC_LT, end);
			TEST_ASSERT(bn_cmp_dig(a, (dig_t)0) == RLC_GT, end);
			TEST_ASSERT(bn_cmp_dig(b, (dig_t)0) == RLC_LT, end);
		} TEST_END;
		bits = 0;
		TEST_CASE("different forms of assignment are consistent") {
			bn_set_dig(a, (dig_t)(1) << (dig_t)bits);
			bn_set_2b(b, bits);
			bits++;
			bits %= (RLC_DIG);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
		} TEST_END;
		TEST_CASE("reading and writing a positive number are consistent") {
			int len = RLC_CEIL(RLC_BN_BITS, 8);
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			for (int j = 2; j <= 64; j++) {
				bits = bn_size_str(a, j);
				bn_write_str(str, bits, a, j);
				bn_read_str(b, str, bits, j);
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
			bn_write_bin(bin, len, a);
			bn_read_bin(b, bin, len);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			len = RLC_BN_DIGS;
			bn_write_raw(raw, len, a);
			bn_read_raw(b, raw, len);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
		}
		TEST_END;
		TEST_CASE("getting the size of a positive number is correct") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			TEST_ASSERT((bn_size_str(a, 2) - 1) == bn_bits(a), end);
			bits = (bn_bits(a) % 8 == 0 ? bn_bits(a) / 8 : bn_bits(a) / 8 + 1);
			TEST_ASSERT(bn_size_bin(a) == bits, end);
			TEST_ASSERT(bn_size_raw(a) == a->used, end);
		}
		TEST_END;
		TEST_CASE("reading and writing a negative number are consistent") {
			int len = RLC_CEIL(RLC_BN_BITS, 8);
			bn_rand(a, RLC_NEG, RLC_BN_BITS);
			for (int j = 2; j <= 64; j++) {
				bits = bn_size_str(a, j);
				bn_write_str(str, bits, a, j);
				bn_read_str(b, str, bits, j);
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
			bn_write_bin(bin, len, a);
			bn_read_bin(b, bin, len);
			bn_neg(b, b);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			len = RLC_BN_DIGS;
			bn_write_raw(raw, len, a);
			bn_read_raw(b, raw, len);
			bn_neg(b, b);
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
		}
		TEST_END;
		TEST_CASE("getting the size of a negative number is correct") {
			bn_rand(a, RLC_NEG, RLC_BN_BITS);
			TEST_ASSERT((bn_size_str(a, 2) - 2) == bn_bits(a), end);
			bits = (bn_bits(a) % 8 == 0 ? bn_bits(a) / 8 : bn_bits(a) / 8 + 1);
			TEST_ASSERT(bn_size_bin(a) == bits, end);
			TEST_ASSERT(bn_size_raw(a) == a->used, end);
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