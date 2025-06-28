static void arith(void) {
	bn_t a, b, c, d[3], e[3];
	crt_t crt;
	dig_t f;
	int len;
	bn_null(a);
	bn_null(b);
	bn_null(c);
	crt_null(crt);
	bn_new(a);
	bn_new(b);
	bn_new(c);
	for (int j = 0; j < 3; j++) {
		bn_null(d[j]);
		bn_null(e[j]);
		bn_new(d[j]);
		bn_new(e[j]);
	}
	crt_new(crt);
	BENCH_RUN("bn_add") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_add(c, a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_add_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		bn_get_dig(&f, b);
		BENCH_ADD(bn_add_dig(c, a, f));
	}
	BENCH_END;
	BENCH_RUN("bn_sub") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_sub(c, a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_sub_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		bn_get_dig(&f, b);
		BENCH_ADD(bn_sub_dig(c, a, f));
	}
	BENCH_END;
	BENCH_RUN("bn_mul") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mul(c, a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_mul_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		bn_get_dig(&f, b);
		BENCH_ADD(bn_mul_dig(c, a, f));
	}
	BENCH_END;
#if BN_MUL == BASIC || !defined(STRIP)
	BENCH_RUN("bn_mul_basic") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mul_basic(c, a, b));
	}
	BENCH_END;
#endif
#if BN_MUL == COMBA || !defined(STRIP)
	BENCH_RUN("bn_mul_comba") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mul_comba(c, a, b));
	}
	BENCH_END;
#endif
#if BN_KARAT > 0 || !defined(STRIP)
	BENCH_RUN("bn_mul_karat") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mul_karat(c, a, b));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_sqr") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_sqr(c, a));
	}
	BENCH_END;
#if BN_SQR == BASIC || !defined(STRIP)
	BENCH_RUN("bn_sqr_basic") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_sqr_basic(c, a));
	}
	BENCH_END;
#endif
#if BN_SQR == COMBA || !defined(STRIP)
	BENCH_RUN("bn_sqr_comba") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_sqr_comba(c, a));
	}
	BENCH_END;
#endif
#if BN_KARAT > 0 || !defined(STRIP)
	BENCH_RUN("bn_sqr_karat") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_sqr_karat(c, a));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_dbl") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_dbl(c, a));
	}
	BENCH_END;
	BENCH_RUN("bn_hlv") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_hlv(c, a));
	}
	BENCH_END;
	BENCH_RUN("bn_lsh") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_lsh(c, a, RLC_BN_BITS / 2 + RLC_DIG / 2));
	}
	BENCH_END;
	BENCH_RUN("bn_rsh") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_rsh(c, a, RLC_BN_BITS / 2 + RLC_DIG / 2));
	}
	BENCH_END;
	BENCH_RUN("bn_div") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_div(c, a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_div_rem") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_div_rem(c, d[0], a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_div_dig") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		do {
			bn_rand(b, RLC_POS, RLC_DIG);
		} while (bn_is_zero(b));
		BENCH_ADD(bn_div_dig(c, a, b->dp[0]));
	}
	BENCH_END;
	BENCH_RUN("bn_div_rem_dig") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		do {
			bn_rand(b, RLC_POS, RLC_DIG);
		} while (bn_is_zero(b));
		BENCH_ADD(bn_div_rem_dig(c, &f, a, b->dp[0]));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_2b") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mod_2b(c, a, RLC_BN_BITS / 2));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		do {
			bn_rand(b, RLC_POS, RLC_DIG);
		} while (bn_is_zero(b));
		BENCH_ADD(bn_mod_dig(&f, a, b->dp[0]));
	}
	BENCH_END;
	BENCH_RUN("bn_mod") {
#if BN_MOD == PMERS
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_set_2b(b, RLC_BN_BITS);
		bn_rand(c, RLC_POS, RLC_DIG);
		bn_sub(b, b, c);
		bn_mod_pre(d, b);
#else
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod_pre(d[0], b);
#endif
		BENCH_ADD(bn_mod(c, a, b, d[0]));
	}
	BENCH_END;
#if BN_MOD == BASIC || !defined(STRIP)
	BENCH_RUN("bn_mod_basic") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mod_basic(c, a, b));
	}
	BENCH_END;
#endif
#if BN_MOD == BARRT || !defined(STRIP)
	BENCH_RUN("bn_mod_pre_barrt") {
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_mod_pre_barrt(d[0], b));
	}
	BENCH_END;
#endif
#if BN_MOD == BARRT || !defined(STRIP)
	BENCH_RUN("bn_mod_barrt") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		bn_mod_pre_barrt(d[0], b);
		BENCH_ADD(bn_mod_barrt(c, a, b, d[0]));
	}
	BENCH_END;
#endif
#if BN_MOD == MONTY || !defined(STRIP)
	BENCH_RUN("bn_mod_pre_monty") {
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		BENCH_ADD(bn_mod_pre_monty(d[0], b));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_monty_conv") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod(a, a, b);
		BENCH_ADD(bn_mod_monty_conv(a, a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_monty") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod(a, a, b);
		bn_mod_pre_monty(d[0], b);
		BENCH_ADD(bn_mod_monty(c, a, b, d[0]));
	}
	BENCH_END;
#if BN_MUL == BASIC || !defined(STRIP)
	BENCH_RUN("bn_mod_monty_basic") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod(a, a, b);
		bn_mod_pre_monty(d[0], b);
		BENCH_ADD(bn_mod_monty_basic(c, a, b, d[0]));
	}
	BENCH_END;
#endif
#if BN_MUL == COMBA || !defined(STRIP)
	BENCH_RUN("bn_mod_monty_comba") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod(a, a, b);
		bn_mod_pre_monty(d[0], b);
		BENCH_ADD(bn_mod_monty_comba(c, a, b, d[0]));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_mod_monty_back") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		bn_mod(a, a, b);
		bn_mod_pre_monty(d[0], b);
		BENCH_ADD(bn_mod_monty_back(c, c, b));
	}
	BENCH_END;
#endif
#if BN_MOD == PMERS || !defined(STRIP)
	BENCH_RUN("bn_mod_pre_pmers") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_set_2b(b, RLC_BN_BITS);
		bn_rand(c, RLC_POS, RLC_DIG);
		bn_sub(b, b, c);
		BENCH_ADD(bn_mod_pre_pmers(d[0], b));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_pmers") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_set_2b(b, RLC_BN_BITS);
		bn_rand(c, RLC_POS, RLC_DIG);
		bn_sub(b, b, c);
		bn_mod_pre_pmers(d[0], b);
		BENCH_ADD(bn_mod_pmers(c, a, b, d[0]));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_mxp") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
#if BN_MOD != PMERS
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
#else
		bn_set_2b(b, RLC_BN_BITS);
		bn_rand(c, RLC_POS, RLC_DIG);
		bn_sub(b, b, c);
#endif
		bn_mod(a, a, b);
		BENCH_ADD(bn_mxp(c, a, b, b));
	}
	BENCH_END;
#if BN_MXP == BASIC || !defined(STRIP)
	BENCH_RUN("bn_mxp_basic") {
		bn_mod(a, a, b);
		BENCH_ADD(bn_mxp_basic(c, a, b, b));
	}
	BENCH_END;
#endif
#if BN_MXP == SLIDE || !defined(STRIP)
	BENCH_RUN("bn_mxp_slide") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_mod(a, a, b);
		BENCH_ADD(bn_mxp_slide(c, a, b, b));
	}
	BENCH_END;
#endif
#if BN_MXP == CONST || !defined(STRIP)
	BENCH_RUN("bn_mxp_monty") {
		bn_rand(a, RLC_POS, 2 * RLC_BN_BITS - RLC_DIG / 2);
		bn_mod(a, a, b);
		BENCH_ADD(bn_mxp_monty(c, a, b, b));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_mxp_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(d[0], RLC_POS, RLC_DIG);
		bn_get_dig(&f, d[0]);
		BENCH_ADD(bn_mxp_dig(c, a, f, b));
	}
	BENCH_END;
	bn_gen_prime(crt->p, RLC_BN_BITS / 2);
	bn_gen_prime(crt->q, RLC_BN_BITS / 2);
	bn_mul(crt->n, crt->p, crt->q);
	bn_mod_inv(crt->qi, crt->q, crt->p);
	bn_sub_dig(crt->dp, crt->p, 1);
	bn_sub_dig(crt->dq, crt->q, 1);
	BENCH_RUN("bn_mxp_crt") {
		bn_rand(c, RLC_POS, RLC_BN_BITS);
		bn_mod(a, c, crt->dp);
		bn_mod(b, c, crt->dq);
		BENCH_ADD(bn_mxp_crt(c, c, a, b, crt, 0));
	}
	BENCH_END;
	BENCH_RUN("bn_srt") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_srt(b, a));
	}
	BENCH_END;
	BENCH_RUN("bn_gcd") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd(c, a, b));
	}
	BENCH_END;
#if BN_GCD == BASIC || !defined(STRIP)
	BENCH_RUN("bn_gcd_basic") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_basic(c, a, b));
	}
	BENCH_END;
#endif
#if BN_GCD == LEHME || !defined(STRIP)
	BENCH_RUN("bn_gcd_lehme") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_lehme(c, a, b));
	}
	BENCH_END;
#endif
#if BN_GCD == BINAR || !defined(STRIP)
	BENCH_RUN("bn_gcd_binar") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_binar(c, a, b));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_gcd_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_DIG);
		bn_get_dig(&f, b);
		BENCH_ADD(bn_gcd_dig(c, a, f));
	}
	BENCH_END;
	BENCH_RUN("bn_gcd_ext") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext(c, d[0], d[1], a, b));
	}
	BENCH_END;
#if BN_GCD == BASIC || !defined(STRIP)
	BENCH_RUN("bn_gcd_ext_basic") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext_basic(c, d[0], d[1], a, b));
	}
	BENCH_END;
#endif
#if BN_GCD == BINAR || !defined(STRIP)
	BENCH_RUN("bn_gcd_ext_binar") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext_binar(c, d[0], d[1], a, b));
	}
	BENCH_END;
#endif
#if BN_GCD == LEHME || !defined(STRIP)
	BENCH_RUN("bn_gcd_ext_lehme") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext_lehme(c, d[0], d[1], a, b));
	}
	BENCH_END;
#endif
#if BN_GCD == BINAR || !defined(STRIP)
	BENCH_RUN("bn_gcd_ext_binar") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext_binar(c, d[0], d[1], a, b));
	}
	BENCH_END;
#endif
	BENCH_RUN("bn_gcd_ext_mid") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_gcd_ext_mid(c, c, d[0], d[1], a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_gcd_ext_dig") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_DIG);
		BENCH_ADD(bn_gcd_ext_dig(c, d[0], d[1], a, b->dp[0]));
	}
	BENCH_END;
	BENCH_RUN("bn_lcm") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_lcm(c, a, b));
	}
	BENCH_END;
	bn_gen_prime(b, RLC_BN_BITS);
	BENCH_RUN("bn_smb_leg") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_smb_leg(a, b));
	}
	BENCH_END;
	BENCH_RUN("bn_smb_jac") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		if (bn_is_even(b)) {
			bn_add_dig(b, b, 1);
		}
		BENCH_ADD(bn_smb_jac(a, b));
	}
	BENCH_END;
	BENCH_ONE("bn_gen_prime", bn_gen_prime(a, RLC_BN_BITS), 1);
#if BN_GEN == BASIC || !defined(STRIP)
	BENCH_ONE("bn_gen_prime_basic", bn_gen_prime_basic(a, RLC_BN_BITS), 1);
#endif
#if BN_GEN == SAFEP || !defined(STRIP)
	BENCH_ONE("bn_gen_prime_safep", bn_gen_prime_safep(a, RLC_BN_BITS), 1);
#endif
#if BN_GEN == STRON || !defined(STRIP)
	BENCH_ONE("bn_gen_prime_stron", bn_gen_prime_stron(a, RLC_BN_BITS), 1);
#endif
	BENCH_ONE("bn_is_prime", bn_is_prime(a), 1);
	BENCH_ONE("bn_is_prime_basic", bn_is_prime_basic(a), 1);
	BENCH_ONE("bn_is_prime_rabin", bn_is_prime_rabin(a), 1);
	BENCH_ONE("bn_is_prime_solov", bn_is_prime_solov(a), 1);
	BENCH_RUN("bn_mod_inv") {
		bn_rand_mod(b, a);
		BENCH_ADD(bn_mod_inv(c, b, a));
	}
	BENCH_END;
	BENCH_RUN("bn_mod_inv_sim (2)") {
		bn_rand_mod(d[0], a);
		bn_rand_mod(d[1], a);
		BENCH_ADD(bn_mod_inv_sim(d, d, a, 2));
	}
	BENCH_END;
	BENCH_RUN("bn_lag (2)") {
		bn_rand_mod(d[0], a);
		bn_rand_mod(d[1], a);
		BENCH_ADD(bn_lag(d, d, a, 2));
	}
	BENCH_END;
	BENCH_RUN("bn_evl (2)") {
		bn_rand_mod(b, a);
		bn_rand_mod(d[0], a);
		bn_rand_mod(d[1], a);
		bn_lag(d, d, a, 2);
		BENCH_ADD(bn_evl(c, d, b, a, 2));
	}
	BENCH_END;
	bn_rand(a, RLC_POS, RLC_BN_BITS);
	BENCH_ONE("bn_factor", bn_factor(c, a), 1);
	BENCH_RUN("bn_is_factor") {
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD(bn_is_factor(b, a));
	}
	BENCH_END;
	BENCH_RUN("bn_rec_win") {
		uint8_t win[RLC_BN_BITS + 1];
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD((len = RLC_BN_BITS + 1, bn_rec_win(win, &len, a, 4)));
	}
	BENCH_END;
	BENCH_RUN("bn_rec_slw") {
		uint8_t win[RLC_BN_BITS + 1];
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD((len = RLC_BN_BITS + 1, bn_rec_slw(win, &len, a, 4)));
	}
	BENCH_END;
	BENCH_RUN("bn_rec_naf") {
		int8_t naf[RLC_BN_BITS + 1];
		int len;
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD((len = RLC_BN_BITS + 1, bn_rec_naf(naf, &len, a, 4)));
	}
	BENCH_END;
#if defined(WITH_EB) && defined(EB_KBLTZ) && (EB_MUL == LWNAF || EB_MUL == RWNAF || EB_FIX == LWNAF || EB_SIM == INTER || !defined(STRIP))
	if (eb_param_set_any_kbltz() == RLC_OK) {
		BENCH_RUN("bn_rec_tnaf") {
			int8_t tnaf[RLC_FB_BITS + 8];
			int len = RLC_BN_BITS + 1;
			eb_curve_get_ord(b);
			bn_rand_mod(a, b);
			if (eb_curve_opt_a() == RLC_ZERO) {
				BENCH_ADD((len = RLC_FB_BITS + 8, bn_rec_tnaf(tnaf, &len, a, -1, RLC_FB_BITS, 4)));
			} else {
				BENCH_ADD((len = RLC_FB_BITS + 8, bn_rec_tnaf(tnaf, &len, a, 1, RLC_FB_BITS, 4)));
			}
		}
		BENCH_END;
		BENCH_RUN("bn_rec_rtnaf") {
			int8_t tnaf[RLC_FB_BITS + 8];
			eb_curve_get_ord(b);
			bn_rand_mod(a, b);
			if (eb_curve_opt_a() == RLC_ZERO) {
				BENCH_ADD((len = RLC_FB_BITS + 8, bn_rec_rtnaf(tnaf, &len, a, -1, RLC_FB_BITS, 4)));
			} else {
				BENCH_ADD((len = RLC_FB_BITS + 8, bn_rec_rtnaf(tnaf, &len, a, 1, RLC_FB_BITS, 4)));
			}
		}
		BENCH_END;
	}
#endif
	BENCH_RUN("bn_rec_reg") {
		int8_t naf[RLC_BN_BITS + 1];
		int len = RLC_BN_BITS + 1;
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		BENCH_ADD((len = RLC_BN_BITS + 1, bn_rec_reg(naf, &len, a, RLC_BN_BITS, 4)));
	}
	BENCH_END;
	BENCH_RUN("bn_rec_jsf") {
		int8_t jsf[2 * (RLC_BN_BITS + 1)];
		bn_rand(a, RLC_POS, RLC_BN_BITS);
		bn_rand(b, RLC_POS, RLC_BN_BITS);
		BENCH_ADD((len = 2 * (RLC_BN_BITS + 1), bn_rec_jsf(jsf, &len, a, b)));
	}
	BENCH_END;
#if defined(WITH_EP) && defined(EP_ENDOM) && (EP_MUL == LWNAF || EP_FIX == COMBS || EP_FIX == LWNAF || EP_SIM == INTER || !defined(STRIP))
	if (ep_param_set_any_endom() == RLC_OK) {
		BENCH_RUN("bn_rec_glv") {
			ep_curve_get_v1(d);
			ep_curve_get_v2(e);
			ep_curve_get_ord(c);
			bn_rand_mod(a, c);
			BENCH_ADD(bn_rec_glv(a, b, a, c, (const bn_t *)d, (const bn_t *)e));
		}
		BENCH_END;
	}
#endif  
	bn_free(a);
	bn_free(b);
	bn_free(c);
	for (int j = 0; j < 3; j++) {
		bn_free(d[j]);
		bn_free(e[j]);
	}
	crt_free(crt);
}