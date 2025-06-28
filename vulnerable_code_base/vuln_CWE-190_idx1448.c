int cp_vbnn_gen_prv(bn_t sk, ec_t pk, const bn_t msk, const uint8_t *id,
		size_t id_len) {
	uint8_t hash[RLC_MD_LEN];
	int len, result = RLC_OK;
	uint8_t *buf = NULL;
	bn_t n, r;
	bn_null(n);
	bn_null(r);
	RLC_TRY {
		bn_new(n);
		bn_new(r);
		ec_curve_get_ord(n);
		bn_rand_mod(r, n);
		ec_mul_gen(pk, r);
		len = id_len + ec_size_bin(pk, 1);
		buf = RLC_ALLOCA(uint8_t, len);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		memcpy(buf, id, id_len);
		ec_write_bin(buf + id_len, ec_size_bin(pk, 1), pk, 1);
		md_map(hash, buf, len);
		bn_read_bin(sk, hash, RLC_MD_LEN);
		bn_mod(sk, sk, n);
		bn_mul(sk, sk, msk);
		bn_add(sk, sk, r);
		bn_mod(sk, sk, n);
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		bn_free(n);
		bn_free(r);
		RLC_FREE(buf);
	}
	return result;
}