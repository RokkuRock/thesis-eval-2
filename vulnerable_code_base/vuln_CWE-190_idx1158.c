void bn_lsh(bn_t c, const bn_t a, int bits) {
	int digits;
	dig_t carry;
	bn_copy(c, a);
	if (bits <= 0) {
		return;
	}
	RLC_RIP(bits, digits, bits);
	RLC_TRY {
		bn_grow(c, c->used + digits + (bits > 0));
		c->used = a->used + digits;
		c->sign = a->sign;
		if (digits > 0) {
			dv_lshd(c->dp, a->dp, c->used, digits);
		}
		if (bits > 0) {
			if (c != a) {
				carry = bn_lshb_low(c->dp + digits, a->dp, a->used, bits);
			} else {
				carry = bn_lshb_low(c->dp + digits, c->dp + digits, c->used - digits, bits);
			}
			if (carry != 0) {
				c->dp[c->used] = carry;
				(c->used)++;
			}
		}
		bn_trim(c);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
}