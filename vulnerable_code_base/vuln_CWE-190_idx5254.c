void bn_write_str(char *str, int len, const bn_t a, int radix) {
	bn_t t;
	dig_t d;
	int digits, l, i, j;
	char c;
	bn_null(t);
	l = bn_size_str(a, radix);
	if (len < l) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	if (radix < 2 || radix > 64) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (bn_is_zero(a) == 1) {
		*str++ = '0';
		*str = '\0';
		return;
	}
	RLC_TRY {
		bn_new(t);
		bn_copy(t, a);
		j = 0;
		if (t->sign == RLC_NEG) {
			str[j] = '-';
			j++;
			t->sign = RLC_POS;
		}
		digits = 0;
		while (!bn_is_zero(t) && j < len) {
			bn_div_rem_dig(t, &d, t, (dig_t)radix);
			str[j] = util_conv_char(d);
			digits++;
			j++;
		}
		i = 0;
		if (str[0] == '-') {
			i = 1;
		}
		j = l - 2;
		while (i < j) {
			c = str[i];
			str[i] = str[j];
			str[j] = c;
			++i;
			--j;
		}
		str[l - 1] = '\0';
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t);
	}
}