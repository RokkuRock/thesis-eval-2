void fb_exp_slide(fb_t c, const fb_t a, const bn_t b) {
	fb_t t[1 << (FB_WIDTH - 1)], r;
	int i, j, l;
	uint8_t win[RLC_FB_BITS + 1];
	fb_null(r);
	if (bn_is_zero(b)) {
		fb_set_dig(c, 1);
		return;
	}
	for (i = 0; i < (1 << (FB_WIDTH - 1)); i++) {
		fb_null(t[i]);
	}
	RLC_TRY {
		for (i = 0; i < (1 << (FB_WIDTH - 1)); i ++) {
			fb_new(t[i]);
		}
		fb_new(r);
		fb_copy(t[0], a);
		fb_sqr(r, a);
		for (i = 1; i < 1 << (FB_WIDTH - 1); i++) {
			fb_mul(t[i], t[i - 1], r);
		}
		fb_set_dig(r, 1);
		l = RLC_FB_BITS + 1;
		bn_rec_slw(win, &l, b, FB_WIDTH);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				fb_sqr(r, r);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					fb_sqr(r, r);
				}
				fb_mul(r, r, t[win[i] >> 1]);
			}
		}
		if (bn_sign(b) == RLC_NEG) {
			fb_inv(c, r);
		} else {
			fb_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (FB_WIDTH - 1)); i++) {
			fb_free(t[i]);
		}
		fb_free(r);
	}
}