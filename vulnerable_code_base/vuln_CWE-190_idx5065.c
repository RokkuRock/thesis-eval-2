void ed_map(ed_t p, const uint8_t *msg, int len) {
	ed_map_dst(p, msg, len, (const uint8_t *)"RELIC", 5);
}