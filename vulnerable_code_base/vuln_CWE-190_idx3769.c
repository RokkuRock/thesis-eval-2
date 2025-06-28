static void ep2_mul_fix_plain(ep2_t r, const ep2_t *table, const bn_t k) {
	int len, i, n;
	int8_t naf[2 * RLC_FP_BITS + 1], *t;
	if (bn_is_zero(k)) {
		ep2_set_infty(r);
		return;
	}
	len = 2 * RLC_FP_BITS + 1;
	bn_rec_naf(naf, &len, k, EP_DEPTH);
	t = naf + len - 1;
	ep2_set_infty(r);
	for (i = len - 1; i >= 0; i--, t--) {
		ep2_dbl(r, r);
		n = *t;
		if (n > 0) {
			ep2_add(r, r, table[n / 2]);
		}
		if (n < 0) {
			ep2_sub(r, r, table[-n / 2]);
		}
	}
	ep2_norm(r, r);
	if (bn_sign(k) == RLC_NEG) {
		ep2_neg(r, r);
	}
}