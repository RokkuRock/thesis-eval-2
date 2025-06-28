void md_hmac(uint8_t *mac, const uint8_t *in, int in_len, const uint8_t *key,
    int key_len) {
#if MD_MAP == SH224 || MD_MAP == SH256 || MD_MAP == B2S160 || MD_MAP == B2S256
  #define block_size 64
#elif MD_MAP == SH384 || MD_MAP == SH512
  #define block_size  128
#endif
    uint8_t opad[block_size + RLC_MD_LEN];
    uint8_t *ipad = RLC_ALLOCA(uint8_t, block_size + in_len);
	uint8_t _key[RLC_MAX(RLC_MD_LEN, block_size)];
    if (ipad == NULL) {
        RLC_THROW(ERR_NO_MEMORY);
		return;
    }
	if (key_len > block_size) {
		md_map(_key, key, key_len);
		key = _key;
		key_len = RLC_MD_LEN;
	}
	if (key_len <= block_size) {
		memcpy(_key, key, key_len);
		memset(_key + key_len, 0, block_size - key_len);
		key = _key;
	}
	for (int i = 0; i < block_size; i++) {
		opad[i] = 0x5C ^ key[i];
		ipad[i] = 0x36 ^ key[i];
	}
	memcpy(ipad + block_size, in, in_len);
	md_map(opad + block_size, ipad, block_size + in_len);
	md_map(mac, opad, block_size + RLC_MD_LEN);
    RLC_FREE(ipad);
}