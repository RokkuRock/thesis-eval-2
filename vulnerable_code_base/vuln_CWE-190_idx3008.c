static void ed_mul_reg_imp(ed_t r, const ed_t p, const bn_t k) {
	bn_t _k;
	int i, j, l, n;
	int8_t s, reg[RLC_CEIL(RLC_FP_BITS + 1, ED_WIDTH - 1)];
	ed_t t[1 << (ED_WIDTH - 2)], u, v;
	bn_null(_k);
	if (bn_is_zero(k)) {
		ed_set_infty(r);
		return;
	}
	RLC_TRY {
		bn_new(_k);
		ed_new(u);
		ed_new(v);
		for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
			ed_null(t[i]);
			ed_new(t[i]);
		}
		ed_tab(t, p, ED_WIDTH);
		bn_abs(_k, k);
		_k->dp[0] |= bn_is_even(_k);
		l = RLC_CEIL(RLC_FP_BITS + 1, ED_WIDTH - 1);
		bn_rec_reg(reg, &l, _k, RLC_FP_BITS, ED_WIDTH);
		ed_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			for (j = 0; j < ED_WIDTH - 1; j++) {
#if ED_ADD == EXTND
				r->coord = EXTND;
#endif
				ed_dbl(r, r);
			}
			n = reg[i];
			s = (n >> 7);
			n = ((n ^ s) - s) >> 1;
			for (j = 0; j < (1 << (EP_WIDTH - 2)); j++) {
				dv_copy_cond(u->x, t[j]->x, RLC_FP_DIGS, j == n);
				dv_copy_cond(u->y, t[j]->y, RLC_FP_DIGS, j == n);
				dv_copy_cond(u->z, t[j]->z, RLC_FP_DIGS, j == n);
			}
			ed_neg(v, u);
			dv_copy_cond(u->x, v->x, RLC_FP_DIGS, s != 0);
			ed_add(r, r, u);
		}
		ed_sub(u, r, t[0]);
		dv_copy_cond(r->x, u->x, RLC_FP_DIGS, bn_is_even(k));
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, bn_is_even(k));
		dv_copy_cond(r->z, u->z, RLC_FP_DIGS, bn_is_even(k));
		ed_norm(r, r);
		ed_neg(u, r);
		dv_copy_cond(r->x, u->x, RLC_FP_DIGS, bn_sign(k) == RLC_NEG);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (ED_WIDTH - 2)); i++) {
			ed_free(t[i]);
		}
		bn_free(_k);
	}
}