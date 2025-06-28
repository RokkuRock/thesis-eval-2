void ep2_mul_sim_joint(ep2_t r, const ep2_t p, const bn_t k, const ep2_t q,
		const bn_t m) {
	bn_t n, _k, _m;
	ep2_t t[5];
	int i, l, u_i, offset;
	int8_t jsf[2 * (RLC_FP_BITS + 1)];
	if (bn_is_zero(k) || ep2_is_infty(p)) {
		ep2_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ep2_is_infty(q)) {
		ep2_mul(r, p, k);
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
			ep2_null(t[i]);
			ep2_new(t[i]);
		}
		ep2_curve_get_ord(n);
		bn_mod(_k, k, n);
		bn_mod(_m, m, n);
		ep2_set_infty(t[0]);
		ep2_copy(t[1], q);
		if (bn_sign(_m) == RLC_NEG) {
			ep2_neg(t[1], t[1]);
		}
		ep2_copy(t[2], p);
		if (bn_sign(_k) == RLC_NEG) {
			ep2_neg(t[2], t[2]);
		}
		ep2_add(t[3], t[2], t[1]);
		ep2_sub(t[4], t[2], t[1]);
#if defined(EP_MIXED)
		ep2_norm_sim(t + 3, t + 3, 2);
#endif
		l = 2 * (RLC_FP_BITS + 1);
		bn_rec_jsf(jsf, &l, _k, _m);
		ep2_set_infty(r);
		offset = RLC_MAX(bn_bits(_k), bn_bits(_m)) + 1;
		for (i = l - 1; i >= 0; i--) {
			ep2_dbl(r, r);
			if (jsf[i] != 0 && jsf[i] == -jsf[i + offset]) {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep2_sub(r, r, t[4]);
				} else {
					ep2_add(r, r, t[4]);
				}
			} else {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep2_sub(r, r, t[-u_i]);
				} else {
					ep2_add(r, r, t[u_i]);
				}
			}
		}
		ep2_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(_k);
		bn_free(_m);
		for (i = 0; i < 5; i++) {
			ep2_free(t[i]);
		}
	}
}