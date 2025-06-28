void bn_make(bn_t a, int digits) {
	if (digits < 0) {
		RLC_THROW(ERR_NO_VALID);
	}
	digits = RLC_MAX(digits, 1);
#if ALLOC == DYNAMIC
	if (digits % RLC_BN_SIZE != 0) {
		digits += (RLC_BN_SIZE - digits % RLC_BN_SIZE);
	}
	if (a != NULL) {
		a->dp = NULL;
#if ALIGN == 1
		a->dp = (dig_t *)malloc(digits * sizeof(dig_t));
#elif OPSYS == WINDOWS
		a->dp = _aligned_malloc(digits * sizeof(dig_t), ALIGN);
#else
		int r = posix_memalign((void **)&a->dp, ALIGN, digits * sizeof(dig_t));
		if (r == ENOMEM) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		if (r == EINVAL) {
			RLC_THROW(ERR_NO_VALID);
		}
#endif  
	}
	if (a->dp == NULL) {
		free((void *)a);
		RLC_THROW(ERR_NO_MEMORY);
	}
#else
	if (digits > RLC_BN_SIZE) {
		RLC_THROW(ERR_NO_PRECI);
		return;
	} else {
		digits = RLC_BN_SIZE;
	}
#endif
	if (a != NULL) {
		a->used = 1;
		a->dp[0] = 0;
		a->alloc = digits;
		a->sign = RLC_POS;
	}
}