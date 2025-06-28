int cp_vbnn_ver(const ec_t r, const bn_t z, const bn_t h, const uint8_t *id,
		size_t id_len, const uint8_t *msg, int msg_len, const ec_t mpk) {
	int len, result = 0;
	uint8_t *buf = NULL, *buf_i, hash[RLC_MD_LEN];
	bn_t n, c, _h;
	ec_t Z;
	ec_t t;
	bn_null(n);
	bn_null(c);
	bn_null(_h);
	ec_null(Z);
	ec_null(t);
	RLC_TRY {
		bn_new(n);
		bn_new(c);
		bn_new(_h);
		ec_new(Z);
		ec_new(t);
		len = id_len + msg_len + 2 * ec_size_bin(r, 1);
		buf = RLC_ALLOCA(uint8_t, len);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		ec_curve_get_ord(n);
		buf_i = buf;
		memcpy(buf_i, id, id_len);
		buf_i += id_len;
		ec_write_bin(buf_i, ec_size_bin(r, 1), r, 1);
		len = id_len + ec_size_bin(r, 1);
		md_map(hash, buf, len);
		bn_read_bin(c, hash, RLC_MD_LEN);
		bn_mod(c, c, n);
		ec_mul_gen(Z, z);
		ec_mul(t, mpk, c);
		ec_add(t, t, r);
		ec_norm(t, t);
		ec_mul(t, t, h);
		ec_sub(Z, Z, t);
		ec_norm(Z, Z);
		buf_i = buf;
		memcpy(buf_i, id, id_len);
		buf_i += id_len;
		memcpy(buf_i, msg, msg_len);
		buf_i += msg_len;
		ec_write_bin(buf_i, ec_size_bin(r, 1), r, 1);
		buf_i += ec_size_bin(r, 1);
		ec_write_bin(buf_i, ec_size_bin(Z, 1), Z, 1);
		len = id_len + msg_len + ec_size_bin(r, 1) + ec_size_bin(Z, 1);
		md_map(hash, buf, len);
		bn_read_bin(_h, hash, RLC_MD_LEN);
		bn_mod(_h, _h, n);
		RLC_FREE(buf);
		if (bn_cmp(h, _h) == RLC_EQ) {
			result = 1;
		} else {
			result = 0;
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(c);
		bn_free(_h);
		ec_free(Z);
		ec_free(t);
		RLC_FREE(buf);
	}
	return result;
}