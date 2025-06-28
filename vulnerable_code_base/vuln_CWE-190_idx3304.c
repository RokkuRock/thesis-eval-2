void fp_exp_slide(fp_t c, const fp_t a, const bn_t b) {
	fp_t t[1 << (FP_WIDTH - 1)], r;
	int i, j, l;
	uint8_t win[RLC_FP_BITS + 1];
	fp_null(r);
	if (bn_is_zero(b)) {
		fp_set_dig(c, 1);
		return;
	}
	for (i = 0; i < (1 << (FP_WIDTH - 1)); i++) {
		fp_null(t[i]);
	}
	RLC_TRY {
		for (i = 0; i < (1 << (FP_WIDTH - 1)); i ++) {
			fp_new(t[i]);
		}
		fp_new(r);
		fp_copy(t[0], a);
		fp_sqr(r, a);
		for (i = 1; i < 1 << (FP_WIDTH - 1); i++) {
			fp_mul(t[i], t[i - 1], r);
		}
		fp_set_dig(r, 1);
		l = RLC_FP_BITS + 1;
		bn_rec_slw(win, &l, b, FP_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				fp_sqr(r, r);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					fp_sqr(r, r);
				}
				fp_mul(r, r, t[win[i] >> 1]);
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
		for (i = 0; i < (1 << (FP_WIDTH - 1)); i++) {
			fp_free(t[i]);
		}
		fp_free(r);
	}
}