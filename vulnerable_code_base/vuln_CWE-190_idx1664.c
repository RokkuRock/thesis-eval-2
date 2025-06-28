static void ep_mul_reg_imp(ep_t r, const ep_t p, const bn_t k) {
	bn_t _k;
	int i, j, l, n;
	int8_t s, reg[1 + RLC_CEIL(RLC_FP_BITS + 1, EP_WIDTH - 1)];
	ep_t t[1 << (EP_WIDTH - 2)], u, v;
	if (bn_is_zero(k)) {
		ep_set_infty(r);
		return;
	}
	bn_null(_k);
	RLC_TRY {
		bn_new(_k);
		ep_new(u);
		ep_new(v);
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		ep_tab(t, p, EP_WIDTH);
		ep_curve_get_ord(_k);
		n = bn_bits(_k);
		bn_abs(_k, k);
		_k->dp[0] |= 1;
		l = RLC_CEIL(n, EP_WIDTH - 1) + 1;
		bn_rec_reg(reg, &l, _k, n, EP_WIDTH);
#if defined(EP_MIXED)
		fp_set_dig(u->z, 1);
		u->coord = BASIC;
#else
		u->coord = EP_ADD;
#endif
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			for (j = 0; j < EP_WIDTH - 1; j++) {
				ep_dbl(r, r);
			}
			n = reg[i];
			s = (n >> 7);
			n = ((n ^ s) - s) >> 1;
			for (j = 0; j < (1 << (EP_WIDTH - 2)); j++) {
				dv_copy_cond(u->x, t[j]->x, RLC_FP_DIGS, j == n);
				dv_copy_cond(u->y, t[j]->y, RLC_FP_DIGS, j == n);
#if !defined(EP_MIXED)
				dv_copy_cond(u->z, t[j]->z, RLC_FP_DIGS, j == n);
#endif
			}
			ep_neg(v, u);
			dv_copy_cond(u->y, v->y, RLC_FP_DIGS, s != 0);
			ep_add(r, r, u);
		}
		ep_sub(u, r, t[0]);
		dv_copy_cond(r->x, u->x, RLC_FP_DIGS, bn_is_even(k));
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, bn_is_even(k));
		dv_copy_cond(r->z, u->z, RLC_FP_DIGS, bn_is_even(k));
		ep_norm(r, r);
		ep_neg(u, r);
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, bn_sign(k) == RLC_NEG);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_free(t[i]);
		}
		bn_free(_k);
		ep_free(u);
		ep_free(v);
	}
}