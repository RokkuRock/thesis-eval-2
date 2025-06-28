void bn_read_raw(bn_t a, const dig_t *raw, int len) {
	RLC_TRY {
		bn_grow(a, len);
		a->used = len;
		a->sign = RLC_POS;
		dv_copy(a->dp, raw, len);
		bn_trim(a);
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
}