void fp_exp_basic(fp_t c, const fp_t a, const bn_t b) {
	int i, l;
	fp_t r;
	fp_null(r);
	if (bn_is_zero(b)) {
		fp_set_dig(c, 1);
		return;
	}
	RLC_TRY {
		fp_new(r);
		l = bn_bits(b);
		fp_copy(r, a);
		for (i = l - 2; i >= 0; i--) {
			fp_sqr(r, r);
			if (bn_get_bit(b, i)) {
				fp_mul(r, r, a);
			}
		}
		if (bn_sign(b) == RLC_NEG) {
			fp_inv(c, r);
		} else {
			fp_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp_free(r);
	}
}