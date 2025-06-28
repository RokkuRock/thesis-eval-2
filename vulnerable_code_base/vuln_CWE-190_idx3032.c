void rand_seed(uint8_t *buf, int size) {
	ctx_t *ctx = core_get();
	int len = (RLC_RAND_SIZE - 1) / 2;
	if (size <= 0) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (sizeof(int) > 4 && size > (1 << 32)) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	ctx->rand[0] = 0x0;
	if (ctx->seeded == 0) {
		rand_hash(ctx->rand + 1, len, buf, size);
		rand_hash(ctx->rand + 1 + len, len, ctx->rand, len + 1);
	} else {
        int tmp_size = 1 + len + size;
		uint8_t* tmp = RLC_ALLOCA(uint8_t, tmp_size);
		if (tmp == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
			return;
		}
		tmp[0] = 1;
		memcpy(tmp + 1, ctx->rand + 1, len);
		memcpy(tmp + 1 + len, buf, size);
		rand_hash(ctx->rand + 1, len, tmp, tmp_size);
		rand_hash(ctx->rand + 1 + len, len, ctx->rand, len + 1);
		RLC_FREE(tmp);
	}
	ctx->counter = ctx->seeded = 1;
}