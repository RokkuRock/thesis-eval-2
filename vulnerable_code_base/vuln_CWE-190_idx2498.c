void ep2_map(ep2_t p, const uint8_t *msg, int len) {
	ep2_map_dst(p, msg, len, (const uint8_t *)"RELIC", 5);
}