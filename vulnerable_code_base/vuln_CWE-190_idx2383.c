int util(void) {
	int l, code = RLC_ERR;
	gt_t a, b, c;
	uint8_t bin[24 * RLC_PC_BYTES];
	gt_null(a);
	gt_null(b);
	gt_null(c);
	RLC_TRY {
		gt_new(a);
		gt_new(b);
		gt_new(c);
		TEST_CASE("comparison is consistent") {
			gt_rand(a);
			gt_rand(b);
			TEST_ASSERT(gt_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;
		TEST_CASE("copy and comparison are consistent") {
			gt_rand(a);
			gt_rand(b);
			gt_rand(c);
			if (gt_cmp(a, c) != RLC_EQ) {
				gt_copy(c, a);
				TEST_ASSERT(gt_cmp(c, a) == RLC_EQ, end);
			}
			if (gt_cmp(b, c) != RLC_EQ) {
				gt_copy(c, b);
				TEST_ASSERT(gt_cmp(b, c) == RLC_EQ, end);
			}
		}
		TEST_END;
		TEST_CASE("inversion and comparison are consistent") {
			gt_rand(a);
			gt_inv(b, a);
			TEST_ASSERT(gt_cmp(a, b) != RLC_EQ, end);
		}
		TEST_END;
		TEST_CASE
				("assignment to random/infinity and comparison are consistent")
		{
			gt_rand(a);
			gt_set_unity(c);
			TEST_ASSERT(gt_cmp(a, c) != RLC_EQ, end);
			TEST_ASSERT(gt_cmp(c, a) != RLC_EQ, end);
		}
		TEST_END;
		TEST_CASE("assignment to unity and unity test are consistent") {
			gt_set_unity(a);
			TEST_ASSERT(gt_is_unity(a), end);
		}
		TEST_END;
	}
	RLC_CATCH_ANY {
		util_print("FATAL ERROR!\n");
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	gt_free(a);
	gt_free(b);
	gt_free(c);
	return code;
}