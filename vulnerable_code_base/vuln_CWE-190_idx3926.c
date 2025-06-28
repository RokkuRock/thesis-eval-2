static void ed_mul_fix_plain(ed_t r, const ed_t * t, const bn_t k) {
	int l, i, n;
	int8_t naf[RLC_FP_BITS + 1], *_k;
	l = RLC_FP_BITS + 1;
	bn_rec_naf(naf, &l, k, ED_DEPTH);
	_k = naf + l - 1;
	ed_set_infty(r);
	for (i = l - 1; i >= 0; i--, _k--) {
		n = *_k;
		if (n == 0) {
			if (i > 0) {
				r->coord = EXTND;
				ed_dbl(r, r);
			} else {
				ed_dbl(r, r);
			}
		} else {
			ed_dbl(r, r);
			if (n > 0) {
				ed_add(r, r, t[n / 2]);
			} else if (n < 0) {
				ed_sub(r, r, t[-n / 2]);
			}
		}
	}
	ed_norm(r, r);
	if (bn_sign(k) == RLC_NEG) {
		ed_neg(r, r);
	}
}