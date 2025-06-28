static void pp_mil_k48(fp48_t r, const fp8_t qx, const fp8_t qy, const ep_t p,
		const bn_t a) {
	fp48_t l;
	ep_t _p;
	fp8_t rx, ry, rz, qn;
	int i, len = bn_bits(a) + 1;
	int8_t s[RLC_FP_BITS + 1];
	fp48_null(l);
	ep_null(_p);
	fp8_null(rx);
	fp8_null(ry);
	fp8_null(rz);
	fp8_null(qn);
	RLC_TRY {
		fp48_new(l);
		ep_new(_p);
		fp8_new(rx);
		fp8_new(ry);
		fp8_new(rz);
		fp8_new(qn);
		fp48_zero(l);
		fp8_copy(rx, qx);
		fp8_copy(ry, qy);
		fp8_set_dig(rz, 1);
#if EP_ADD == BASIC
		ep_neg(_p, p);
#else
		fp_add(_p->x, p->x, p->x);
		fp_add(_p->x, _p->x, p->x);
		fp_neg(_p->y, p->y);
#endif
		fp8_neg(qn, qy);
		bn_rec_naf(s, &len, a, 2);
		for (i = len - 2; i >= 0; i--) {
			fp48_sqr(r, r);
			pp_dbl_k48(l, rx, ry, rz, _p);
			fp48_mul_dxs(r, r, l);
			if (s[i] > 0) {
				pp_add_k48(l, rx, ry, rz, qx, qy, p);
				fp48_mul_dxs(r, r, l);
			}
			if (s[i] < 0) {
				pp_add_k48(l, rx, ry, rz, qx, qn, p);
				fp48_mul_dxs(r, r, l);
			}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fp48_free(l);
		ep_free(_p);
		fp8_free(rx);
		fp8_free(ry);
		fp8_free(rz);
		fp8_free(qn);
	}
}