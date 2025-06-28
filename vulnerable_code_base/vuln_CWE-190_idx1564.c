void ep_map(ep_t p, const uint8_t *msg, int len) {
	ep_map_dst(p, msg, len, (const uint8_t *)"RELIC", 5);
}