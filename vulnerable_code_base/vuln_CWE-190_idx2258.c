void md_map_sh224(uint8_t *hash, const uint8_t *msg, int len) {
	SHA224Context ctx;
	if (SHA224Reset(&ctx) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA224Input(&ctx, msg, len) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA224Result(&ctx, hash) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}