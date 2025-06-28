void ep4_mul_sim_joint(ep4_t r, const ep4_t p, const bn_t k, const ep4_t q,
		const bn_t m) {
	ep4_t t[5];
	int i, l, u_i, offset;
	int8_t jsf[4 * (RLC_FP_BITS + 1)];
	if (bn_is_zero(k) || ep4_is_infty(p)) {
		ep4_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ep4_is_infty(q)) {
		ep4_mul(r, p, k);
		return;
	}
	RLC_TRY {
		for (i = 0; i < 5; i++) {
			ep4_null(t[i]);
			ep4_new(t[i]);
		}
		ep4_set_infty(t[0]);
		ep4_copy(t[1], q);
		if (bn_sign(m) == RLC_NEG) {
			ep4_neg(t[1], t[1]);
		}
		ep4_copy(t[2], p);
		if (bn_sign(k) == RLC_NEG) {
			ep4_neg(t[2], t[2]);
		}
		ep4_add(t[3], t[2], t[1]);
		ep4_sub(t[4], t[2], t[1]);
#if defined(EP_MIXED)
		ep4_norm_sim(t + 3, t + 3, 2);
#endif
		l = 4 * (RLC_FP_BITS + 1);
		bn_rec_jsf(jsf, &l, k, m);
		ep4_set_infty(r);
		offset = RLC_MAX(bn_bits(k), bn_bits(m)) + 1;
		for (i = l - 1; i >= 0; i--) {
			ep4_dbl(r, r);
			if (jsf[i] != 0 && jsf[i] == -jsf[i + offset]) {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep4_sub(r, r, t[4]);
				} else {
					ep4_add(r, r, t[4]);
				}
			} else {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ep4_sub(r, r, t[-u_i]);
				} else {
					ep4_add(r, r, t[u_i]);
				}
			}
		}
		ep4_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < 5; i++) {
			ep4_free(t[i]);
		}
	}
}