void fb_write_str(char *str, int len, const fb_t a, int radix) {
	fb_t t;
	int d, l, i, j;
	char c;
	fb_null(t);
	l = fb_size_str(a, radix);
	if (len < l) {
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	len = l;
	l = log_radix(radix);
	if (!valid_radix(radix)) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (fb_is_zero(a) == 1) {
		*str++ = '0';
		*str = '\0';
		return;
	}
	RLC_TRY {
		fb_new(t);
		fb_copy(t, a);
		j = 0;
		while (!fb_is_zero(t)) {
			d = t[0] % radix;
			fb_rshb_low(t, t, l);
			str[j] = util_conv_char(d);
			j++;
		}
		i = 0;
		j = len - 2;
		while (i < j) {
			c = str[i];
			str[i] = str[j];
			str[j] = c;
			++i;
			--j;
		}
		str[len - 1] = '\0';
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		fb_free(t);
	}
}