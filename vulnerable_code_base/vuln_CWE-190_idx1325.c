void eb_mul_halve(eb_t r, const eb_t p, const bn_t k) {
	int i, j, l, trc, cof;
	int8_t naf[RLC_FB_BITS + 1], *_k;
	eb_t q, s, t[1 << (EB_WIDTH - 2)];
	bn_t n, m;
	fb_t u, v, w, z;
	if (bn_is_zero(k) || eb_is_infty(p)) {
		eb_set_infty(r);
		return;
	}
	bn_null(m);
	bn_null(n);
	eb_null(q);
	eb_null(s);
	for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
		eb_null(t[i]);
	}
	fb_null(u);
	fb_null(v);
	fb_null(w);
	fb_null(z);
	RLC_TRY {
		bn_new(n);
		bn_new(m);
		eb_new(q);
		eb_new(s);
		fb_new(u);
		fb_new(v);
		fb_new(w);
		fb_new(z);
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_new(t[i]);
			eb_set_infty(t[i]);
		}
		eb_curve_get_ord(n);
		bn_lsh(m, k, bn_bits(n) - 1);
		bn_mod(m, m, n);
		l = sizeof(naf);
		bn_rec_naf(naf, &l, m, EB_WIDTH);
		if (naf[bn_bits(n)] == 1) {
			eb_dbl(t[0], p);
		}
		l = bn_bits(n);
		_k = naf + l - 1;
		eb_copy(q, p);
		eb_curve_get_cof(n);
		if (bn_cmp_dig(n, 2) == RLC_GT) {
			cof = 1;
		} else {
			cof = 0;
		}
		trc = fb_trc(eb_curve_get_a());
		if (cof) {
			fb_srt(u, eb_curve_get_a());
			fb_slv(v, u);
			bn_rand(n, RLC_POS, l);
			for (i = l - 1; i >= 0; i--, _k--) {
				j = *_k;
				if (j > 0) {
					eb_norm(s, q);
					eb_add(t[j / 2], t[j / 2], s);
				}
				if (j < 0) {
					eb_norm(s, q);
					eb_sub(t[-j / 2], t[-j / 2], s);
				}
				eb_hlv(s, q);
				if (fb_trc(s->x) != 0) {
					fb_copy(z, s->y);
					fb_srt(w, q->y);
					fb_add(s->y, s->y, w);
					fb_add(s->y, s->y, v);
					fb_add(z, z, q->x);
					fb_add(z, z, v);
					fb_add(z, z, u);
					fb_add(w, w, q->x);
					fb_add(w, w, q->y);
					fb_add(w, w, u);
					fb_mul(w, w, z);
					fb_srt(s->x, w);
					fb_set_dig(s->z, 1);
					s->coord = HALVE;
				}
				eb_copy(q, s);
			}
		} else {
			for (i = l - 1; i >= 0; i--, _k--) {
				j = *_k;
				if (j > 0) {
					eb_norm(q, q);
					eb_add(t[j / 2], t[j / 2], q);
				}
				if (j < 0) {
					eb_norm(q, q);
					eb_sub(t[-j / 2], t[-j / 2], q);
				}
				eb_hlv(q, q);
			}
		}
#if EB_WIDTH == 2
		eb_norm(r, t[0]);
#else
		for (i = (1 << (EB_WIDTH - 1)) - 3; i >= 1; i -= 2) {
			eb_add(t[i / 2], t[i / 2], t[(i + 2) / 2]);
		}
		eb_copy(r, t[1]);
		for (i = 2; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_add(r, r, t[i]);
		}
		eb_dbl(r, r);
		eb_add(r, r, t[0]);
		eb_norm(r, r);
#endif
		if (cof) {
			eb_hlv(s, r);
			if (fb_trc(s->x) != trc) {
				fb_zero(s->x);
				fb_srt(s->y, eb_curve_get_b());
				fb_set_dig(s->z, 1);
				eb_add(r, r, s);
				eb_norm(r, r);
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		for (i = 0; i < (1 << (EB_WIDTH - 2)); i++) {
			eb_free(t[i]);
		}
		bn_free(n);
		bn_free(m);
		eb_free(q);
		eb_free(s);
		fb_free(u);
		fb_free(v);
		fb_free(w);
		fb_free(z);
	}
}