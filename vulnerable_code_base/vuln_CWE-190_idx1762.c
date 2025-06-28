void bn_rsh(bn_t c, const bn_t a, int bits) {
	int digits = 0;
	bn_copy(c, a);
	if (bits <= 0) {
		return;
	}
	RLC_RIP(bits, digits, bits);
	if (digits > 0) {
		dv_rshd(c->dp, a->dp, a->used, digits);
	}
	c->used = a->used - digits;
	c->sign = a->sign;
	if (c->used > 0 && bits > 0) {
		if (digits == 0 && c != a) {
			bn_rshb_low(c->dp, a->dp + digits, a->used - digits, bits);
		} else {
			bn_rshb_low(c->dp, c->dp, c->used, bits);
		}
	}
	bn_trim(c);
}