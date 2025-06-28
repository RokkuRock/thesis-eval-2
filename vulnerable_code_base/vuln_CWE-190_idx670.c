void md_map_sh256(uint8_t *hash, const uint8_t *msg, int len) {
	SHA256Context ctx;
	if (SHA256Reset(&ctx) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA256Input(&ctx, msg, len) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA256Result(&ctx, hash) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}