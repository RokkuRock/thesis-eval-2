static void rand_hash(uint8_t *out, int out_len, uint8_t *in, int in_len) {
	uint32_t j = util_conv_big(8 * out_len);
	int len = RLC_CEIL(out_len, RLC_MD_LEN);
	uint8_t* buf = RLC_ALLOCA(uint8_t, 1 + sizeof(uint32_t) + in_len);
	uint8_t hash[RLC_MD_LEN];
	if (buf == NULL) {
		RLC_THROW(ERR_NO_MEMORY);
		return;
	}
	buf[0] = 1;
	memcpy(buf + 1, &j, sizeof(uint32_t));
	memcpy(buf + 1 + sizeof(uint32_t), in, in_len);
	for (int i = 0; i < len; i++) {
		md_map(hash, buf, 1 + sizeof(uint32_t) + in_len);
		memcpy(out, hash, RLC_MIN(RLC_MD_LEN, out_len));
		out += RLC_MD_LEN;
		out_len -= RLC_MD_LEN;
		buf[0]++;
	}
	RLC_FREE(buf);
}