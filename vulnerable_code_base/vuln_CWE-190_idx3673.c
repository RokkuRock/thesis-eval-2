static void ep4_mul_fix_ordin(ep4_t r, const ep4_t *table, const bn_t k) {
	int len, i, n;
	int8_t naf[2 * RLC_FP_BITS + 1], *t;
	if (bn_is_zero(k)) {
		ep4_set_infty(r);
		return;
	}
	len = 2 * RLC_FP_BITS + 1;
	bn_rec_naf(naf, &len, k, EP_DEPTH);
	t = naf + len - 1;
	ep4_set_infty(r);
	for (i = len - 1; i >= 0; i--, t--) {
		ep4_dbl(r, r);
		n = *t;
		if (n > 0) {
			ep4_add(r, r, table[n / 2]);
		}
		if (n < 0) {
			ep4_sub(r, r, table[-n / 2]);
		}
	}
	ep4_norm(r, r);
	if (bn_sign(k) == RLC_NEG) {
		ep4_neg(r, r);
	}
}