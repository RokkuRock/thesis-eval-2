static void pp_mil_k8(fp8_t r, ep2_t *t, ep2_t *q, ep_t *p, int m, bn_t a) {
	fp8_t l;
	ep_t *_p = RLC_ALLOCA(ep_t, m);
	ep2_t *_q = RLC_ALLOCA(ep2_t, m);
	int i, j, len = bn_bits(a) + 1;
	int8_t s[RLC_FP_BITS + 1];
	if (m == 0) {
		return;
	}
	fp8_null(l);
	RLC_TRY {
		fp8_new(l);
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
			fp_neg(_p[j]->x, p[j]->x);
			fp_copy(_p[j]->y, p[j]->y);
#endif
		}
		fp8_zero(l);
		bn_rec_naf(s, &len, a, 2);
		for (i = len - 2; i >= 0; i--) {
			fp8_sqr(r, r);
			for (j = 0; j < m; j++) {
				pp_dbl_k8(l, t[j], t[j], _p[j]);
				fp8_mul(r, r, l);
				if (s[i] > 0) {
					pp_add_k8(l, t[j], q[j], _p[j]);
					fp8_mul_dxs(r, r, l);
				}
				if (s[i] < 0) {
					pp_add_k8(l, t[j], _q[j], _p[j]);
					fp8_mul_dxs(r, r, l);
				}
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp8_free(l);
		for (j = 0; j < m; j++) {
			ep_free(_p[j]);
			ep2_free(_q[j]);
		}
		RLC_FREE(_p);
		RLC_FREE(_q);
	}
}