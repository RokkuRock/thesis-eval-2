void bn_rec_jsf(int8_t *jsf, int *len, const bn_t k, const bn_t l) {
	bn_t n0, n1;
	dig_t l0, l1;
	int8_t u0, u1, d0, d1;
	int i, j, offset;
	if (*len < (2 * bn_bits(k) + 1)) {
		*len = 0;
		RLC_THROW(ERR_NO_BUFFER);
		return;
	}
	bn_null(n0);
	bn_null(n1);
	RLC_TRY {
		bn_new(n0);
		bn_new(n1);
		bn_abs(n0, k);
		bn_abs(n1, l);
		i = bn_bits(k);
		j = bn_bits(l);
		offset = RLC_MAX(i, j) + 1;
		memset(jsf, 0, *len);
		i = 0;
		d0 = d1 = 0;
		while (!(bn_is_zero(n0) && d0 == 0) || !(bn_is_zero(n1) && d1 == 0)) {
			bn_get_dig(&l0, n0);
			bn_get_dig(&l1, n1);
			l0 = (l0 + d0) & RLC_MASK(3);
			l1 = (l1 + d1) & RLC_MASK(3);
			if (l0 % 2 == 0) {
				u0 = 0;
			} else {
				u0 = 2 - (l0 & RLC_MASK(2));
				if ((l0 == 3 || l0 == 5) && ((l1 & RLC_MASK(2)) == 2)) {
					u0 = (int8_t)-u0;
				}
			}
			jsf[i] = u0;
			if (l1 % 2 == 0) {
				u1 = 0;
			} else {
				u1 = 2 - (l1 & RLC_MASK(2));
				if ((l1 == 3 || l1 == 5) && ((l0 & RLC_MASK(2)) == 2)) {
					u1 = (int8_t)-u1;
				}
			}
			jsf[i + offset] = u1;
			if (d0 + d0 == 1 + u0) {
				d0 = (int8_t)(1 - d0);
			}
			if (d1 + d1 == 1 + u1) {
				d1 = (int8_t)(1 - d1);
			}
			i++;
			bn_hlv(n0, n0);
			bn_hlv(n1, n1);
		}
		*len = i;
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n0);
		bn_free(n1);
	}
}