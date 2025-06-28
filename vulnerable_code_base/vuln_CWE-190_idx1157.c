void ep_mul_monty(ep_t r, const ep_t p, const bn_t k) {
	int i, j, bits;
	ep_t t[2];
	bn_t n, l, _k;
	bn_null(n);
	bn_null(l);
	bn_null(_k);
	ep_null(t[0]);
	ep_null(t[1]);
	if (bn_is_zero(k) || ep_is_infty(p)) {
		ep_set_infty(r);
		return;
	}
	RLC_TRY {
		bn_new(n);
		bn_new(l);
		bn_new(_k);
		ep_new(t[0]);
		ep_new(t[1]);
		ep_curve_get_ord(n);
		bits = bn_bits(n);
		bn_mod(_k, k, n);
		bn_abs(l, _k);
		bn_add(l, l, n);
		bn_add(n, l, n);
		dv_swap_cond(l->dp, n->dp, RLC_MAX(l->used, n->used),
			bn_get_bit(l, bits) == 0);
		l->used = RLC_SEL(l->used, n->used, bn_get_bit(l, bits) == 0);
		ep_norm(t[0], p);
		ep_dbl(t[1], t[0]);
		ep_blind(t[0], t[0]);
		ep_blind(t[1], t[1]);
		for (i = bits - 1; i >= 0; i--) {
			j = bn_get_bit(l, i);
			dv_swap_cond(t[0]->x, t[1]->x, RLC_FP_DIGS, j ^ 1);
			dv_swap_cond(t[0]->y, t[1]->y, RLC_FP_DIGS, j ^ 1);
			dv_swap_cond(t[0]->z, t[1]->z, RLC_FP_DIGS, j ^ 1);
			ep_add(t[0], t[0], t[1]);
			ep_dbl(t[1], t[1]);
			dv_swap_cond(t[0]->x, t[1]->x, RLC_FP_DIGS, j ^ 1);
			dv_swap_cond(t[0]->y, t[1]->y, RLC_FP_DIGS, j ^ 1);
			dv_swap_cond(t[0]->z, t[1]->z, RLC_FP_DIGS, j ^ 1);
		}
		ep_norm(r, t[0]);
		ep_neg(t[0], r);
		dv_copy_cond(r->y, t[0]->y, RLC_FP_DIGS, bn_sign(_k) == RLC_NEG);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(l);
		bn_free(_k);
		ep_free(t[1]);
		ep_free(t[0]);
	}
}