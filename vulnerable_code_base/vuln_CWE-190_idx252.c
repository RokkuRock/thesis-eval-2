void rand_bytes(uint8_t *buf, int size) {
	uint8_t hash[RLC_MD_LEN];
	int carry, len  = (RLC_RAND_SIZE - 1)/2;
	ctx_t *ctx = core_get();
	if (sizeof(int) > 2 && size > (1 << 16)) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	rand_gen(buf, size);
	ctx->rand[0] = 0x3;
	md_map(hash, ctx->rand, 1 + len);
	rand_add(ctx->rand + 1, ctx->rand + 1 + len, len);
	carry = rand_add(ctx->rand + 1 + (len - RLC_MD_LEN), hash, RLC_MD_LEN);
	rand_inc(ctx->rand, len - RLC_MD_LEN + 1, carry);
	rand_inc(ctx->rand, len + 1, ctx->counter);
	ctx->counter = ctx->counter + 1;
}