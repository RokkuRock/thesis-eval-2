void fp_read_bin(fp_t a, const uint8_t *bin, int len) {
	bn_t t;
	bn_null(t);
	if (len != RLC_FP_BYTES) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	RLC_TRY {
		bn_new(t);
		bn_read_bin(t, bin, len);
		if (bn_sign(t) == RLC_NEG || bn_cmp(t, &core_get()->prime) != RLC_LT) {
			RLC_THROW(ERR_NO_VALID);
		} else {
			if (bn_is_zero(t)) {
				fp_zero(a);
			} else {
				if (t->used == 1) {
					fp_prime_conv_dig(a, t->dp[0]);
				} else {
					fp_prime_conv(a, t);
				}
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