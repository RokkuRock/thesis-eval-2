static int recoding(void) {
	int code = RLC_ERR;
	bn_t a, b, c, v1[3], v2[3];
	int w, k, l;
	uint8_t d[RLC_BN_BITS + 1];
	int8_t e[2 * (RLC_BN_BITS + 1)];
	bn_null(a);
	bn_null(b);
	bn_null(c);
	for (k = 0; k < 3; k++) {
		bn_null(v1[k]);
		bn_null(v2[k]);
	}
	RLC_TRY {
		bn_new(a);
		bn_new(b);
		bn_new(c);
		for (k = 0; k < 3; k++) {
			bn_new(v1[k]);
			bn_new(v2[k]);
		}
		TEST_CASE("window recoding is correct") {
			for (w = 2; w <= 8; w++) {
				bn_rand(a, RLC_POS, RLC_BN_BITS);
				l = RLC_BN_BITS + 1;
				bn_rec_win(d, &l, a, w);
				bn_zero(b);
				for (k = l - 1; k >= 0; k--) {
					bn_lsh(b, b, w);
					bn_add_dig(b, b, d[k]);
				}
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		} TEST_END;
		TEST_CASE("sliding window recoding is correct") {
			for (w = 2; w <= 8; w++) {
				bn_rand(a, RLC_POS, RLC_BN_BITS);
				l = RLC_BN_BITS + 1;
				bn_rec_slw(d, &l, a, w);
				bn_zero(b);
				for (k = 0; k < l; k++) {
					if (d[k] == 0) {
						bn_dbl(b, b);
					} else {
						bn_lsh(b, b, util_bits_dig(d[k]));
						bn_add_dig(b, b, d[k]);
					}
				}
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		} TEST_END;
		TEST_CASE("naf recoding is correct") {
			for (w = 2; w <= 8; w++) {
				bn_rand(a, RLC_POS, RLC_BN_BITS);
				l = RLC_BN_BITS + 1;
				bn_rec_naf(e, &l, a, w);
				bn_zero(b);
				for (k = l - 1; k >= 0; k--) {
					bn_dbl(b, b);
					if (e[k] >= 0) {
						bn_add_dig(b, b, e[k]);
					} else {
						bn_sub_dig(b, b, -e[k]);
					}
				}
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		} TEST_END;
#if defined(WITH_EB) && defined(EB_KBLTZ) && (EB_MUL == LWNAF || EB_MUL == RWNAF || EB_FIX == LWNAF || EB_SIM == INTER || !defined(STRIP))
		if (eb_param_set_any_kbltz() == RLC_OK) {
			eb_curve_get_ord(v1[2]);
			TEST_CASE("tnaf recoding is correct") {
				for (w = 2; w <= 8; w++) {
					uint8_t t_w;
					int8_t beta[64], gama[64];
					int8_t tnaf[RLC_FB_BITS + 8];
					int8_t u = (eb_curve_opt_a() == RLC_ZERO ? -1 : 1);
					bn_rand_mod(a, v1[2]);
					l = RLC_FB_BITS + 1;
					bn_rec_tnaf_mod(v1[0], v1[1], a, u, RLC_FB_BITS);
					bn_rec_tnaf_get(&t_w, beta, gama, u, w);
					bn_rec_tnaf(tnaf, &l, a, u, RLC_FB_BITS, w);
					bn_zero(a);
					bn_zero(b);
					for (k = l - 1; k >= 0; k--) {
						bn_copy(c, b);
						if (u == -1) {
							bn_neg(c, c);
						}
						bn_add(c, c, a);
						bn_dbl(a, b);
						bn_neg(a, a);
						bn_copy(b, c);
						if (w == 2) {
							if (tnaf[k] >= 0) {
								bn_add_dig(a, a, tnaf[k]);
							} else {
								bn_sub_dig(a, a, -tnaf[k]);
							}
						} else {
							if (tnaf[k] > 0) {
								if (beta[tnaf[k] / 2] >= 0) {
									bn_add_dig(a, a, beta[tnaf[k] / 2]);
								} else {
									bn_sub_dig(a, a, -beta[tnaf[k] / 2]);
								}
								if (gama[tnaf[k] / 2] >= 0) {
									bn_add_dig(b, b, gama[tnaf[k] / 2]);
								} else {
									bn_sub_dig(b, b, -gama[tnaf[k] / 2]);
								}
							}
							if (tnaf[k] < 0) {
								if (beta[-tnaf[k] / 2] >= 0) {
									bn_sub_dig(a, a, beta[-tnaf[k] / 2]);
								} else {
									bn_add_dig(a, a, -beta[-tnaf[k] / 2]);
								}
								if (gama[-tnaf[k] / 2] >= 0) {
									bn_sub_dig(b, b, gama[-tnaf[k] / 2]);
								} else {
									bn_add_dig(b, b, -gama[-tnaf[k] / 2]);
								}
							}
						}
					}
					TEST_ASSERT(bn_cmp(a, v1[0]) == RLC_EQ, end);
					TEST_ASSERT(bn_cmp(b, v1[1]) == RLC_EQ, end);
				}
			}
			TEST_END;
			TEST_CASE("regular tnaf recoding is correct") {
				for (w = 2; w <= 8; w++) {
					uint8_t t_w;
					int8_t beta[64], gama[64];
					int8_t tnaf[RLC_FB_BITS + 8];
					int8_t u = (eb_curve_opt_a() == RLC_ZERO ? -1 : 1);
					int n;
					do {
						bn_rand_mod(a, v1[2]);
						l = RLC_FB_BITS + 1;
						bn_rec_tnaf_mod(v1[0], v1[1], a, u, RLC_FB_BITS);
					} while (bn_is_even(v1[0]) || bn_is_even(v1[1]));
					bn_rec_tnaf_get(&t_w, beta, gama, u, w);
					bn_rec_rtnaf(tnaf, &l, a, u, RLC_FB_BITS, w);
					bn_zero(a);
					bn_zero(b);
					n = 0;
					for (k = l - 1; k >= 0; k--) {
						for (int m = 0; m < w - 1; m++) {
							bn_copy(c, b);
							if (u == -1) {
								bn_neg(c, c);
							}
							bn_add(c, c, a);
							bn_dbl(a, b);
							bn_neg(a, a);
							bn_copy(b, c);
						}
						if (tnaf[k] != 0) {
							n++;
						}
						if (w == 2) {
							if (tnaf[k] >= 0) {
								bn_add_dig(a, a, tnaf[k]);
							} else {
								bn_sub_dig(a, a, -tnaf[k]);
							}
						} else {
							if (tnaf[k] > 0) {
								if (beta[tnaf[k] / 2] >= 0) {
									bn_add_dig(a, a, beta[tnaf[k] / 2]);
								} else {
									bn_sub_dig(a, a, -beta[tnaf[k] / 2]);
								}
								if (gama[tnaf[k] / 2] >= 0) {
									bn_add_dig(b, b, gama[tnaf[k] / 2]);
								} else {
									bn_sub_dig(b, b, -gama[tnaf[k] / 2]);
								}
							}
							if (tnaf[k] < 0) {
								if (beta[-tnaf[k] / 2] >= 0) {
									bn_sub_dig(a, a, beta[-tnaf[k] / 2]);
								} else {
									bn_add_dig(a, a, -beta[-tnaf[k] / 2]);
								}
								if (gama[-tnaf[k] / 2] >= 0) {
									bn_sub_dig(b, b, gama[-tnaf[k] / 2]);
								} else {
									bn_add_dig(b, b, -gama[-tnaf[k] / 2]);
								}
							}
						}
					}
					TEST_ASSERT(bn_cmp(a, v1[0]) == RLC_EQ, end);
					TEST_ASSERT(bn_cmp(b, v1[1]) == RLC_EQ, end);
				}
			} TEST_END;
		}
#endif
		TEST_CASE("regular recoding is correct") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			if (bn_is_even(a)) {
				bn_add_dig(a, a, 1);
			}
			for (w = 2; w <= 8; w++) {
				l = RLC_BN_BITS + 1;
				bn_rec_reg(e, &l, a, RLC_BN_BITS, w);
				bn_zero(b);
				for (k = l - 1; k >= 0; k--) {
					bn_lsh(b, b, w - 1);
					if (e[k] > 0) {
						bn_add_dig(b, b, e[k]);
					} else {
						bn_sub_dig(b, b, -e[k]);
					}
				}
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		} TEST_END;
		TEST_CASE("jsf recoding is correct") {
			bn_rand(a, RLC_POS, RLC_BN_BITS);
			bn_rand(b, RLC_POS, RLC_BN_BITS);
			l = 2 * (RLC_BN_BITS + 1);
			bn_rec_jsf(e, &l, a, b);
			w = RLC_MAX(bn_bits(a), bn_bits(b)) + 1;
			bn_add(a, a, b);
			bn_zero(b);
			for (k = l - 1; k >= 0; k--) {
				bn_dbl(b, b);
				if (e[k] >= 0) {
					bn_add_dig(b, b, e[k]);
				} else {
					bn_sub_dig(b, b, -e[k]);
				}
				if (e[k + w] >= 0) {
					bn_add_dig(b, b, e[k + w]);
				} else {
					bn_sub_dig(b, b, -e[k + w]);
				}
			}
			TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
		} TEST_END;
#if defined(WITH_EP) && defined(EP_ENDOM) && (EP_MUL == LWNAF || EP_FIX == COMBS || EP_FIX == LWNAF || EP_SIM == INTER || !defined(STRIP))
		TEST_CASE("glv recoding is correct") {
			if (ep_param_set_any_endom() == RLC_OK) {
				ep_curve_get_v1(v1);
				ep_curve_get_v2(v2);
				ep_curve_get_ord(b);
				bn_rand_mod(a, b);
				bn_rec_glv(b, c, a, b, (const bn_t *)v1, (const bn_t *)v2);
				ep_curve_get_ord(v2[0]);
				TEST_ASSERT(bn_bits(b) <= 1 + (bn_bits(v2[0]) >> 1), end);
				TEST_ASSERT(bn_bits(c) <= 1 + (bn_bits(v2[0]) >> 1), end);
				if (bn_cmp_dig(v1[2], 1) == RLC_EQ) {
					bn_gcd_ext(v1[0], v2[1], NULL, v1[1], v2[0]);
				} else {
					bn_gcd_ext(v1[0], v2[1], NULL, v1[2], v2[0]);
				}
				if (bn_sign(v2[1]) == RLC_NEG) {
					bn_add(v2[1], v2[0], v2[1]);
				}
				if (bn_cmp_dig(v1[2], 1) == RLC_EQ) {
					bn_sub(v1[0], v2[1], v1[2]);
				} else {
					bn_mul(v1[0], v2[1], v1[1]);
				}
				bn_mod(v1[0], v1[0], v2[0]);
				bn_sub(v1[1], v2[0], v1[0]);
				if (bn_cmp(v1[1], v1[0]) == RLC_LT) {
					bn_copy(v1[0], v1[1]);
				}
				bn_mul(c, c, v1[0]);
				bn_add(b, b, c);
				bn_mod(b, b, v2[0]);
				if (bn_sign(b) == RLC_NEG) {
					bn_add(b, b, v2[0]);
				}
				TEST_ASSERT(bn_cmp(a, b) == RLC_EQ, end);
			}
		} TEST_END;
#endif  
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	bn_free(a);
	bn_free(b);
	bn_free(c);
	for (k = 0; k < 3; k++) {
		bn_free(v1[k]);
		bn_free(v2[k]);
	}
	return code;
}