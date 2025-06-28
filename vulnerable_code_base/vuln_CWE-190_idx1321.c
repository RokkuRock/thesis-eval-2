void eb_mul_sim_joint(eb_t r, const eb_t p, const bn_t k, const eb_t q,
		const bn_t m) {
	eb_t t[5];
	int i, u_i, len, offset;
	int8_t jsf[2 * (RLC_FB_BITS + 1)];
	if (bn_is_zero(k) || eb_is_infty(p)) {
		eb_mul(r, q, m);
		return;
	}
	if (bn_is_zero(m) || eb_is_infty(q)) {
		eb_mul(r, p, k);
		return;
	}
	RLC_TRY {
		for (i =  0; i < 5; i++) {
			eb_null(t[i]);
			eb_new(t[i]);
		}
		eb_set_infty(t[0]);
		eb_copy(t[1], q);
		if (bn_sign(m) == RLC_NEG) {
			eb_neg(t[1], t[1]);
		}
		eb_copy(t[2], p);
		if (bn_sign(k) == RLC_NEG) {
			eb_neg(t[2], t[2]);
		}
		eb_add(t[3], t[2], t[1]);
		eb_sub(t[4], t[2], t[1]);
#if defined(EB_MIXED)
		eb_norm_sim(t + 3, (const eb_t*)(t + 3), 2);
#endif
		len = 2 * (RLC_FB_BITS + 1);
		bn_rec_jsf(jsf, &len, k, m);
		eb_set_infty(r);
		offset = RLC_MAX(bn_bits(k), bn_bits(m)) + 1;
		for (i = len - 1; i >= 0; i--) {
			eb_dbl(r, r);
			if (jsf[i] != 0 && jsf[i] == -jsf[i + offset]) {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					eb_sub(r, r, t[4]);
				} else {
					eb_add(r, r, t[4]);
				}
			} else {
				u_i = jsf[i] * 2 + jsf[i + offset];
				if (u_i < 0) {
					eb_sub(r, r, t[-u_i]);
				} else {
					eb_add(r, r, t[u_i]);
				}
			}
		}
		eb_norm(r, r);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i =  0; i < 5; i++) {
			eb_free(t[i]);
		}
	}
}