void md_map_b2s256(uint8_t *hash, const uint8_t *msg, int len) {
	memset(hash, 0, RLC_MD_LEN_B2S256);
	blake2s(hash, RLC_MD_LEN_B2S256, msg, len, NULL, 0);
}