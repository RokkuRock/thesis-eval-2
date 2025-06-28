void bn_read_str(bn_t a, const char *str, int len, int radix) {
	int sign, i, j;
	char c;
	bn_zero(a);
	if (radix < 2 || radix > 64) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	j = 0;
	if (str[0] == '-') {
		j++;
		sign = RLC_NEG;
	} else {
		sign = RLC_POS;
	}
	RLC_TRY {
		bn_grow(a, RLC_CEIL(len * util_bits_dig(radix), RLC_DIG));
		while (j < len) {
			if (str[j] == 0) {
				break;
			}
			c = (char)((radix < 36) ? RLC_UPP(str[j]) : str[j]);
			for (i = 0; i < 64; i++) {
				if (c == util_conv_char(i)) {
					break;
				}
			}
			if (i < radix) {
				bn_mul_dig(a, a, (dig_t)radix);
				bn_add_dig(a, a, (dig_t)i);
			} else {
				break;
			}
			j++;
		}
		a->sign = sign;
		bn_trim(a);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
}