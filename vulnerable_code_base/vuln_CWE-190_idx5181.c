void md_map_sh512(uint8_t *hash, const uint8_t *msg, int len) {
	SHA512Context ctx;
	if (SHA512Reset(&ctx) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA512Input(&ctx, msg, len) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA512Result(&ctx, hash) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}