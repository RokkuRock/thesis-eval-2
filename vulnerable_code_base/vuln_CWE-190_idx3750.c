void bn_grow(bn_t a, int digits) {
#if ALLOC == DYNAMIC
	dig_t *t;
	if (a->alloc < digits) {
		digits += (RLC_BN_SIZE * 2) - (digits % RLC_BN_SIZE);
		t = (dig_t *)realloc(a->dp, (RLC_DIG / 8) * digits);
		if (t == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
			return;
		}
		a->dp = t;
		a->alloc = digits;
	}
#elif ALLOC == AUTO
	if (digits > RLC_BN_SIZE) {
		RLC_THROW(ERR_NO_PRECI);
		return;
	}
	(void)a;
#endif
}