int cp_vbnn_sig(ec_t r, bn_t z, bn_t h, const uint8_t *id, size_t id_len,
		const uint8_t *msg, int msg_len, const bn_t sk, const ec_t pk) {
	int len, result = RLC_OK;
	uint8_t *buf = NULL, *buf_i, hash[RLC_MD_LEN];
	bn_t n, y;
	ec_t t;
	bn_null(n);
	bn_null(y);
	ec_null(t);
	RLC_TRY {
		bn_new(n);
		bn_new(y);
		ec_new(t);
		ec_curve_get_ord(n);
		bn_rand_mod(y, n);
		ec_mul_gen(t, y);
		len = id_len + msg_len + ec_size_bin(t, 1) + ec_size_bin(pk, 1);
		buf = RLC_ALLOCA(uint8_t, len);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		buf_i = buf;
		memcpy(buf_i, id, id_len);
		buf_i += id_len;
		memcpy(buf_i, msg, msg_len);
		buf_i += msg_len;
		ec_write_bin(buf_i, ec_size_bin(pk, 1), pk, 1);
		buf_i += ec_size_bin(pk, 1);
		ec_write_bin(buf_i, ec_size_bin(t, 1), t, 1);
		md_map(hash, buf, len);
		bn_read_bin(h, hash, RLC_MD_LEN);
		bn_mod(h, h, n);
		bn_mul(z, h, sk);
		bn_add(z, z, y);
		bn_mod(z, z, n);
		ec_copy(r, pk);
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(y);
		ec_free(t);
		RLC_FREE(buf);
	}
	return result;
}