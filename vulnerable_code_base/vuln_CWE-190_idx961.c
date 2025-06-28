static void rand_gen(uint8_t *out, int out_len) {
	int m = RLC_CEIL(out_len, RLC_MD_LEN);
	uint8_t hash[RLC_MD_LEN], data[(RLC_RAND_SIZE - 1)/2];
	ctx_t *ctx = core_get();
	memcpy(data, ctx->rand + 1, (RLC_RAND_SIZE - 1)/2);
	for (int i = 0; i < m; i++) {
		md_map(hash, data, sizeof(data));
		memcpy(out, hash, RLC_MIN(RLC_MD_LEN, out_len));
		out += RLC_MD_LEN;
		out_len -= RLC_MD_LEN;
		rand_inc(data, (RLC_RAND_SIZE - 1)/2, 1);
	}
}