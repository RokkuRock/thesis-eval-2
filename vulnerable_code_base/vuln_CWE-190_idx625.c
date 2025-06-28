void ed_mul_sim_joint(ed_t r, const ed_t p, const bn_t k, const ed_t q,
		const bn_t m) {
	ed_t t[5];
	int i, l, u_i, offset;
	int8_t jsf[2 * (RLC_FP_BITS + 1)];
	if (bn_is_zero(k) || ed_is_infty(p)) {
		ed_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || ed_is_infty(q)) {
		ed_mul(r, p, k);
		return;
	}
	RLC_TRY {
		for (i = 0; i < 5; i++) {
			ed_null(t[i]);
			ed_new(t[i]);
		}
		ed_set_infty(t[0]);
		ed_copy(t[1], q);
		if (bn_sign(m) == RLC_NEG) {
			ed_neg(t[1], t[1]);
		}
		ed_copy(t[2], p);
		if (bn_sign(k) == RLC_NEG) {
			ed_neg(t[2], t[2]);
		}
		ed_add(t[3], t[2], t[1]);
		ed_sub(t[4], t[2], t[1]);
#if defined(ED_MIXED)
		ed_norm_sim(t + 3, (const ed_t *)t + 3, 2);
#endif
		l = 2 * (RLC_FP_BITS + 1);
		bn_rec_jsf(jsf, &l, k, m);
		ed_set_infty(r);
		offset = RLC_MAX(bn_bits(k), bn_bits(m)) + 1;
		for (i = l - 1; i >= 0; i--) {
			ed_dbl(r, r);
			if (jsf[i] != 0 && jsf[i] == -jsf[i + offset]) {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ed_sub(r, r, t[4]);
				} else {
					ed_add(r, r, t[4]);
				}
			} else {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					ed_sub(r, r, t[-u_i]);
				} else {
					ed_add(r, r, t[u_i]);
				}
			}
		}
		ed_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < 5; i++) {
			ed_free(t[i]);
		}
	}
}