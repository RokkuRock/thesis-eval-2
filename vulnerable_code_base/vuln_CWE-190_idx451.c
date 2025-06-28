static void pp_mil_k12(fp12_t r, ep2_t *t, ep2_t *q, ep_t *p, int m, bn_t a) {
	fp12_t l;
	ep_t *_p = RLC_ALLOCA(ep_t, m);
	ep2_t *_q = RLC_ALLOCA(ep2_t, m);
	int i, j, len = bn_bits(a) + 1;
	int8_t s[RLC_FP_BITS + 1];
	if (m == 0) {
		return;
	}
	fp12_null(l);
	RLC_TRY {
		fp12_new(l);
		if (_p == NULL || _q == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		for (j = 0; j < m; j++) {
			ep_null(_p[j]);
			ep2_null(_q[j]);
			ep_new(_p[j]);
			ep2_new(_q[j]);
			ep2_copy(t[j], q[j]);
			ep2_neg(_q[j], q[j]);
#if EP_ADD == BASIC
			ep_neg(_p[j], p[j]);
#else
			fp_add(_p[j]->x, p[j]->x, p[j]->x);
			fp_add(_p[j]->x, _p[j]->x, p[j]->x);
			fp_neg(_p[j]->y, p[j]->y);
#endif
		}
		fp12_zero(l);
		bn_rec_naf(s, &len, a, 2);
		pp_dbl_k12(r, t[0], t[0], _p[0]);
		for (j = 1; j < m; j++) {
			pp_dbl_k12(l, t[j], t[j], _p[j]);
			fp12_mul_dxs(r, r, l);
		}
		if (s[len - 2] > 0) {
			for (j = 0; j < m; j++) {
				pp_add_k12(l, t[j], q[j], p[j]);
				fp12_mul_dxs(r, r, l);
			}
		}
		if (s[len - 2] < 0) {
			for (j = 0; j < m; j++) {
				pp_add_k12(l, t[j], _q[j], p[j]);
				fp12_mul_dxs(r, r, l);
			}
		}
		for (i = len - 3; i >= 0; i--) {
			fp12_sqr(r, r);
			for (j = 0; j < m; j++) {
				pp_dbl_k12(l, t[j], t[j], _p[j]);
				fp12_mul_dxs(r, r, l);
				if (s[i] > 0) {
					pp_add_k12(l, t[j], q[j], p[j]);
					fp12_mul_dxs(r, r, l);
				}
				if (s[i] < 0) {
					pp_add_k12(l, t[j], _q[j], p[j]);
					fp12_mul_dxs(r, r, l);
				}
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp12_free(l);
		for (j = 0; j < m; j++) {
			ep_free(_p[j]);
			ep2_free(_q[j]);
		}
		RLC_FREE(_p);
		RLC_FREE(_q);
	}
}