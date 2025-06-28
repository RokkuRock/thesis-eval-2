void ep_mul_sim_joint(ep_t r, const ep_t p, const bn_t k, const ep_t q,
		const bn_t m) {
	bn_t n, _k, _m;
	ep_t t[5];
	int i, l, u_i, offset;
	int8_t jsf[2 * (RLC_FP_BITS + 1)];
	if (bn_is_zero(k) || ep_is_infty(p)) {
		ep_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ep_is_infty(q)) {
		ep_mul(r, p, k);
		return;
	}
	bn_null(n);
	bn_null(_k);
	bn_null(_m);
	RLC_TRY {
		bn_new(n);
		bn_new(_k);
		bn_new(_m);
		for (i = 0; i < 5; i++) {
			ep_null(t[i]);
			ep_new(t[i]);
		}
		ep_curve_get_ord(n);
		bn_mod(_k, k, n);
		bn_mod(_m, m, n);
		ep_set_infty(t[0]);
		ep_copy(t[1], q);
		if (bn_sign(_m) == RLC_NEG) {
			ep_neg(t[1], t[1]);
		}
		ep_copy(t[2], p);
		if (bn_sign(_k) == RLC_NEG) {
			ep_neg(t[2], t[2]);
		}
		ep_add(t[3], t[2], t[1]);
		ep_sub(t[4], t[2], t[1]);
#if defined(EP_MIXED)
		ep_norm_sim(t + 3, (const ep_t *)t + 3, 2);
#endif
		l = 2 * (RLC_FP_BITS + 1);
		bn_rec_jsf(jsf, &l, _k, _m);
		ep_set_infty(r);
		offset = RLC_MAX(bn_bits(_k), bn_bits(_m)) + 1;
		for (i = l - 1; i >= 0; i--) {
			ep_dbl(r, r);
			if (jsf[i] != 0 && jsf[i] == -jsf[i + offset]) {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep_sub(r, r, t[4]);
				} else {
					ep_add(r, r, t[4]);
				}
			} else {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep_sub(r, r, t[-u_i]);
				} else {
					ep_add(r, r, t[u_i]);
				}
			}
		}
		ep_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		bn_free(_m);
		for (i = 0; i < 5; i++) {
			ep_free(t[i]);
		}
	}
}