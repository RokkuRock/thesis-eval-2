void bn_mxp_slide(bn_t c, const bn_t a, const bn_t b, const bn_t m) {
	bn_t tab[RLC_TABLE_SIZE], t, u, r;
	int i, j, l, w = 1;
	uint8_t *win = RLC_ALLOCA(uint8_t, bn_bits(b));
	if (win == NULL) {
		RLC_THROW(ERR_NO_MEMORY);
		return;
	}
	if (bn_cmp_dig(m, 1) == RLC_EQ) {
		RLC_FREE(win);
		bn_zero(c);
		return;
	}
	if (bn_is_zero(b)) {
		RLC_FREE(win);
		bn_set_dig(c, 1);
		return;
	}
	bn_null(t);
	bn_null(u);
	bn_null(r);
	for (i = 0; i < RLC_TABLE_SIZE; i++) {
		bn_null(tab[i]);
	}
	i = bn_bits(b);
	if (i <= 21) {
		w = 2;
	} else if (i <= 32) {
		w = 3;
	} else if (i <= 128) {
		w = 4;
	} else if (i <= 256) {
		w = 5;
	} else if (i <= 512) {
		w = 6;
	} else {
		w = 7;
	}
	RLC_TRY {
		for (i = 0; i < (1 << (w - 1)); i++) {
			bn_new(tab[i]);
		}
		bn_new(t);
		bn_new(u);
		bn_new(r);
		bn_mod_pre(u, m);
#if BN_MOD == MONTY
		bn_set_dig(r, 1);
		bn_mod_monty_conv(r, r, m);
		bn_mod_monty_conv(t, a, m);
#else  
		bn_set_dig(r, 1);
		bn_copy(t, a);
#endif
		bn_copy(tab[0], t);
		bn_sqr(t, tab[0]);
		bn_mod(t, t, m, u);
		for (i = 1; i < 1 << (w - 1); i++) {
			bn_mul(tab[i], tab[i - 1], t);
			bn_mod(tab[i], tab[i], m, u);
		}
		l = bn_bits(b);
		bn_rec_slw(win, &l, b, w);
		for (i = 0; i < l; i++) {
			if (win[i] == 0) {
				bn_sqr(r, r);
				bn_mod(r, r, m, u);
			} else {
				for (j = 0; j < util_bits_dig(win[i]); j++) {
					bn_sqr(r, r);
					bn_mod(r, r, m, u);
				}
				bn_mul(r, r, tab[win[i] >> 1]);
				bn_mod(r, r, m, u);
			}
		}
		bn_trim(r);
#if BN_MOD == MONTY
		bn_mod_monty_back(r, r, m);
#endif
		if (bn_sign(b) == RLC_NEG) {
			bn_mod_inv(c, r, m);
		} else {
			bn_copy(c, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (w - 1)); i++) {
			bn_free(tab[i]);
		}
		bn_free(u);
		bn_free(t);
		bn_free(r);
		RLC_FREE(win);
	}
}