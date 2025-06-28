void fp_read_str(fp_t a, const char *str, int len, int radix) {
	bn_t t;
	bn_null(t);
	RLC_TRY {
		bn_new(t);
		bn_read_str(t, str, len, radix);
		if (bn_is_zero(t)) {
			fp_zero(a);
		} else {
			if (t->used == 1) {
				fp_prime_conv_dig(a, t->dp[0]);
				if (bn_sign(t) == RLC_NEG) {
					fp_neg(a, a);
				}
			} else {
				fp_prime_conv(a, t);
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}