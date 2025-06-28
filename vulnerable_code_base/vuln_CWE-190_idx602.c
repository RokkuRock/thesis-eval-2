static void ep_mul_fix_plain(ep_t r, const ep_t *t, const bn_t k) {
	int l, i, n;
	int8_t naf[RLC_FP_BITS + 1];
	l = RLC_FP_BITS + 1;
	bn_rec_naf(naf, &l, k, EP_DEPTH);
	n = naf[l - 1];
	if (n > 0) {
		ep_copy(r, t[n / 2]);
	} else {
		ep_neg(r, t[-n / 2]);
	}
	for (i = l - 2; i >= 0; i--) {
		ep_dbl(r, r);
		n = naf[i];
		if (n > 0) {
			ep_add(r, r, t[n / 2]);
		}
		if (n < 0) {
			ep_sub(r, r, t[-n / 2]);
		}
	}
	ep_norm(r, r);
	if (bn_sign(k) == RLC_NEG) {
		ep_neg(r, r);
	}
}