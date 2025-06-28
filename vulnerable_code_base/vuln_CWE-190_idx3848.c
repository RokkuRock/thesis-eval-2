static void ep_mul_reg_glv(ep_t r, const ep_t p, const bn_t k) {
	int i, j, l, n0, n1, s0, s1, b0, b1;
	int8_t _s0, _s1, reg0[RLC_FP_BITS + 1], reg1[RLC_FP_BITS + 1];
	bn_t n, _k, k0, k1, v1[3], v2[3];
	ep_t q, t[1 << (EP_WIDTH - 2)], u, v, w;
	bn_null(n);
	bn_null(_k);
	bn_null(k0);
	bn_null(k1);
	ep_null(q);
	ep_null(u);
	ep_null(v);
	ep_null(w);
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		bn_new(k0);
		bn_new(k1);
		ep_new(q);
		ep_new(u);
		ep_new(v);
		ep_new(w);
		for (i = 0; i < (1 << (EP_WIDTH - 2)); i++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		for (i = 0; i < 3; i++) {
			bn_null(v1[i]);
			bn_null(v2[i]);
			bn_new(v1[i]);
			bn_new(v2[i]);
		}
		ep_curve_get_ord(n);
		ep_curve_get_v1(v1);
		ep_curve_get_v2(v2);
		bn_abs(_k, k);
		bn_mod(_k, _k, n);
		bn_rec_glv(k0, k1, _k, n, (const bn_t *)v1, (const bn_t *)v2);
		s0 = bn_sign(k0);
		s1 = bn_sign(k1);
		bn_abs(k0, k0);
		bn_abs(k1, k1);
		b0 = bn_is_even(k0);
		b1 = bn_is_even(k1);
		k0->dp[0] |= b0;
		k1->dp[0] |= b1;
		ep_copy(q, p);
		ep_neg(t[0], p);
		dv_copy_cond(q->y, t[0]->y, RLC_FP_DIGS, s0 != RLC_POS);
		ep_tab(t, q, EP_WIDTH);
		l = RLC_FP_BITS + 1;
		bn_rec_reg(reg0, &l, k0, bn_bits(n)/2, EP_WIDTH);
		l = RLC_FP_BITS + 1;
		bn_rec_reg(reg1, &l, k1, bn_bits(n)/2, EP_WIDTH);
#if defined(EP_MIXED)
		fp_set_dig(u->z, 1);
		fp_set_dig(w->z, 1);
		u->coord = w->coord = BASIC;
#else
		u->coord = w->coord = EP_ADD;
#endif
		ep_set_infty(r);
		for (i = l - 1; i >= 0; i--) {
			for (j = 0; j < EP_WIDTH - 1; j++) {
				ep_dbl(r, r);
			}
			n0 = reg0[i];
			_s0 = (n0 >> 7);
			n0 = ((n0 ^ _s0) - _s0) >> 1;
			n1 = reg1[i];
			_s1 = (n1 >> 7);
			n1 = ((n1 ^ _s1) - _s1) >> 1;
			for (j = 0; j < (1 << (EP_WIDTH - 2)); j++) {
				dv_copy_cond(u->x, t[j]->x, RLC_FP_DIGS, j == n0);
				dv_copy_cond(w->x, t[j]->x, RLC_FP_DIGS, j == n1);
				dv_copy_cond(u->y, t[j]->y, RLC_FP_DIGS, j == n0);
				dv_copy_cond(w->y, t[j]->y, RLC_FP_DIGS, j == n1);
#if !defined(EP_MIXED)
				dv_copy_cond(u->z, t[j]->z, RLC_FP_DIGS, j == n0);
				dv_copy_cond(w->z, t[j]->z, RLC_FP_DIGS, j == n1);
#endif
			}
			ep_neg(v, u);
			dv_copy_cond(u->y, v->y, RLC_FP_DIGS, _s0 != 0);
			ep_add(r, r, u);
			ep_psi(w, w);
			ep_neg(q, w);
			dv_copy_cond(w->y, q->y, RLC_FP_DIGS, s0 != s1);
			ep_neg(q, w);
			dv_copy_cond(w->y, q->y, RLC_FP_DIGS, _s1 != 0);
			ep_add(r, r, w);
		}
		ep_sub(u, r, t[0]);
		dv_copy_cond(r->x, u->x, RLC_FP_DIGS, b0);
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, b0);
		dv_copy_cond(r->z, u->z, RLC_FP_DIGS, b0);
		ep_psi(w, t[0]);
		ep_neg(q, w);
		dv_copy_cond(w->y, q->y, RLC_FP_DIGS, s0 != s1);
		ep_sub(u, r, w);
		dv_copy_cond(r->x, u->x, RLC_FP_DIGS, b1);
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, b1);
		dv_copy_cond(r->z, u->z, RLC_FP_DIGS, b1);
		ep_norm(r, r);
		ep_neg(u, r);
		dv_copy_cond(r->y, u->y, RLC_FP_DIGS, bn_sign(k) == RLC_NEG);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		bn_free(k0);
		bn_free(k1);
		bn_free(n);
		ep_free(q);
		ep_free(u);
		ep_free(v);
		ep_free(w);
		for (i = 0; i < 1 << (EP_WIDTH - 2); i++) {
			ep_free(t[i]);
		}
		for (i = 0; i < 3; i++) {
			bn_free(v1[i]);
			bn_free(v2[i]);
		}
	}
}