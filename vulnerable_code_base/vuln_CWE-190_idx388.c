int cp_cmlhs_sig(g1_t sig, g2_t z, g1_t a, g1_t c, g1_t r, g2_t s,
		const bn_t msg, const char *data, int label, const bn_t x, const g1_t h,
		const uint8_t prf[], size_t plen, const bn_t d, const bn_t sk,
		int bls) {
	bn_t k, m, n;
	g1_t t;
	uint8_t mac[RLC_MD_LEN];
	int len, dlen = strlen(data), result = RLC_OK;
	uint8_t *buf = RLC_ALLOCA(uint8_t, 1 + 8 * RLC_PC_BYTES + dlen);
	bn_null(k);
	bn_null(m);
	bn_null(n);
	g1_null(t);
	RLC_TRY {
		bn_new(k);
		bn_new(m);
		bn_new(n);
		g1_new(t);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		pc_get_ord(n);
		bn_rand_mod(k, n);
		bn_rand_mod(m, n);
		g2_mul_gen(s, m);
		g2_neg(s, s);
		g1_mul_gen(c, m);
		bn_mul(m, d, m);
		bn_mod(m, m, n);
		bn_sub(m, k, m);
		bn_mod(m, m, n);
		g1_mul_gen(r, m);
		bn_add(k, x, k);
		bn_mod(k, k, n);
		g1_mul_gen(a, k);
		bn_mul(k, d, msg);
		bn_mod(k, k, n);
		g1_mul(t, h, k);
		g1_add(a, a, t);
		g1_norm(a, a);
		md_hmac(mac, (const uint8_t *)data, dlen, prf, plen);
		bn_read_bin(k, mac, RLC_MD_LEN);
		bn_mod(k, k, n);
		g2_mul_gen(z, k);
		bn_mod_inv(k, k, n);
		g1_mul(a, a, k);
		bn_mod(k, msg, n);
		g1_mul(t, h, k);
		g1_add(c, c, t);
		g1_norm(c, c);
		len = g2_size_bin(z, 0);
		g2_write_bin(buf, len, z, 0);
		memcpy(buf + len, data, dlen);
		if (bls) {
			cp_bls_sig(sig, buf, len + dlen, sk);
		} else {
			cp_ecdsa_sig(m, n, buf, len + dlen, 0, sk);
			fp_prime_conv(sig->x, m);
			fp_prime_conv(sig->y, n);
			fp_set_dig(sig->z, 1);
		}
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		bn_free(k);
		bn_free(m);
		bn_free(n);
		g1_free(t);
		RLC_FREE(buf);
	}
	return result;
}