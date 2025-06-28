void md_map_sh384(uint8_t *hash, const uint8_t *msg, int len) {
	SHA384Context ctx;
	if (SHA384Reset(&ctx) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA384Input(&ctx, msg, len) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
	if (SHA384Result(&ctx, hash) != shaSuccess) {
		RLC_THROW(ERR_NO_VALID);
		return;
	}
}