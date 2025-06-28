void fp_prime_set_pairf(const bn_t x, int pairf) {
	bn_t p, t0, t1;
	ctx_t *ctx = core_get();
	int len = bn_bits(x) + 1;
	int8_t s[RLC_FP_BITS + 1];
	bn_null(p);
	bn_null(t0);
	bn_null(t1);
	RLC_TRY {
		bn_new(p);
		bn_new(t0);
		bn_new(t1);
		bn_copy(&(ctx->par), x);
		bn_copy(t0, x);
		switch (pairf) {
			case EP_BN:
				bn_set_dig(p, 1);
				bn_mul_dig(t1, t0, 6);
				bn_add(p, p, t1);
				bn_mul(t1, t0, t0);
				bn_mul_dig(t1, t1, 24);
				bn_add(p, p, t1);
				bn_mul(t1, t0, t0);
				bn_mul(t1, t1, t0);
				bn_mul_dig(t1, t1, 36);
				bn_add(p, p, t1);
				bn_mul(t0, t0, t0);
				bn_mul(t1, t0, t0);
				bn_mul_dig(t1, t1, 36);
				bn_add(p, p, t1);
				fp_prime_set_dense(p);
				break;
			case EP_B12:
				bn_sqr(t1, t0);
				bn_sqr(p, t1);
				bn_sub(p, p, t1);
				bn_add_dig(p, p, 1);
				bn_sub(t1, t1, t0);
				bn_sub(t1, t1, t0);
				bn_add_dig(t1, t1, 1);
				bn_mul(p, p, t1);
				bn_div_dig(p, p, 3);
				bn_add(p, p, t0);
				fp_prime_set_dense(p);
				break;
			case EP_OT8:
				bn_set_dig(p, 4);
				bn_mul_dig(t1, t0, 4);
				bn_add(p, p, t1);
				bn_sqr(t0, t0);
				bn_add(p, p, t0);
				bn_sqr(t1, t0);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_div_dig(p, p, 4);
				fp_prime_set_dense(p);
				break;
			case EP_B24:
				bn_sqr(t1, t0);
				bn_sqr(t1, t1);
				bn_sqr(p, t1);
				bn_sub(p, p, t1);
				bn_add_dig(p, p, 1);
				bn_sub_dig(t1, t0, 1);
				bn_sqr(t1, t1);
				bn_mul(p, p, t1);
				bn_div_dig(p, p, 3);
				bn_add(p, p, t0);
				fp_prime_set_dense(p);
				break;
			case EP_B48:
				bn_sqr(t1, t0);
				bn_sqr(t1, t1);
				bn_sqr(p, t1);
				bn_sqr(t1, p);
				bn_sub(t1, t1, p);
				bn_add_dig(t1, t1, 1);
				bn_sub_dig(p, t0, 1);
				bn_sqr(p, p);
				bn_mul(p, p, t1);
				bn_div_dig(p, p, 3);
				bn_add(p, p, t0);
				fp_prime_set_dense(p);
				break;
			case EP_K54:
				bn_set_dig(p, 1);
				bn_mul_dig(t1, t0, 3);
				bn_add(p, p, t1);
				bn_sqr(t1, t0);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_add(p, p, t1);
				bn_sqr(t1, t1);
				bn_sqr(t1, t1);
				bn_mul(t1, t1, t0);
				bn_mul_dig(t1, t1, 243);
				bn_add(p, p, t1);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_mul_dig(t1, t1, 3);
				bn_add(p, p, t1);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_mul_dig(t1, t1, 27);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_mul_dig(t1, t1, 3);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				bn_mul(t1, t1, t0);
				bn_add(p, p, t1);
				fp_prime_set_dense(p);
				break;
		}
		ctx->par_len = 0;
		bn_rec_naf(s, &len, &(ctx->par), 2);
		if (s[0] == -1) {
			s[0] = 1;
			s[1] = -1;
		}
		for (int i = 0; i < len && ctx->par_len < RLC_TERMS; i++) {
			if (s[i] > 0) {
				ctx->par_sps[ctx->par_len++] = i;
			}
			if (s[i] < 0) {
				ctx->par_sps[ctx->par_len++] = -i;
			}
		}
		if (ctx->par_len == RLC_TERMS) {
			RLC_THROW(ERR_NO_VALID);
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		bn_free(p);
		bn_free(t0);
		bn_free(t1);
	}
}