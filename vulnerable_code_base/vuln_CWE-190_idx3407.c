void fp_prime_set_pmers(const int *f, int len) {
	bn_t p, t;
	bn_null(p);
	bn_null(t);
	RLC_TRY {
		bn_new(p);
		bn_new(t);
		if (len >= RLC_TERMS) {
			RLC_THROW(ERR_NO_VALID);
			return;
		}
		bn_set_2b(p, f[len - 1]);
		for (int i = len - 2; i > 0; i--) {
			if (f[i] > 0) {
				bn_set_2b(t, f[i]);
				bn_add(p, p, t);
			} else {
				bn_set_2b(t, -f[i]);
				bn_sub(p, p, t);
			}
		}
		if (f[0] > 0) {
			bn_add_dig(p, p, f[0]);
		} else {
			bn_sub_dig(p, p, -f[0]);
		}
#if FP_RDC == QUICK || !defined(STRIP)
		ctx_t *ctx = core_get();
		for (int i = 0; i < len; i++) {
			ctx->sps[i] = f[i];
		}
		ctx->sps[len] = 0;
		ctx->sps_len = len;
#endif  
		fp_prime_set(p);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(p);
		bn_free(t);
	}
}