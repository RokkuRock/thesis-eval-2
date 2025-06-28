int cp_cmlhs_onv(const g1_t r, const g2_t s, const g1_t sig[], const g2_t z[],
		const g1_t a[], const g1_t c[], const bn_t msg, const char *data,
		const g1_t h, const gt_t vk, const g2_t y[], const g2_t pk[],
		size_t slen, int bls) {
	g1_t g1;
	g2_t g2;
	gt_t e, u, v;
	bn_t k, n;
	int len, dlen = strlen(data), result = 1;
	uint8_t *buf = RLC_ALLOCA(uint8_t, 1 + 8 * RLC_FP_BYTES + dlen);
	g1_null(g1);
	g2_null(g2);
	gt_null(e);
	gt_null(u);
	gt_null(v);
	bn_null(k);
	bn_null(n);
	RLC_TRY {
		g1_new(g1);
		g2_new(g2);
		gt_new(e);
		gt_new(u);
		gt_new(v);
		bn_new(k);
		bn_new(n);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		for (int i = 0; i < slen; i++) {
			len = g2_size_bin(z[i], 0);
			g2_write_bin(buf, len, z[i], 0);
			memcpy(buf + len, data, dlen);
			if (bls) {
				result &= cp_bls_ver(sig[i], buf, len + dlen, pk[i]);
			} else {
				fp_prime_back(k, sig[i]->x);
				fp_prime_back(n, sig[i]->y);
				fp_copy(g1->x, pk[i]->x[0]);
				fp_copy(g1->y, pk[i]->y[0]);
				fp_set_dig(g1->z, 1);
				result &= cp_ecdsa_ver(k, n, buf, len + dlen, 0, g1);
			}
		}
		pc_get_ord(n);
		g1_get_gen(g1);
		g2_get_gen(g2);
		pc_map_sim(e, a, z, slen);
		pc_map_sim(u, c, y, slen);
		pc_map(v, r, g2);
		gt_mul(u, u, v);
		gt_mul(u, u, vk);
		if (gt_cmp(e, u) != RLC_EQ) {
			result = 0;
		}
		pc_map(e, g1, s);
		g1_set_infty(g1);
		for (int i = 0; i < slen; i++) {
			g1_add(g1, g1, c[i]);
		}
		g1_norm(g1, g1);
		pc_map(u, g1, g2);
		gt_mul(e, e, u);
		g1_mul(g1, h, msg);
		pc_map(v, g1, g2);
		if (gt_cmp(e, v) != RLC_EQ) {
			result = 0;
		}
	} RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	} RLC_FINALLY {
		g1_free(g1);
		g2_free(g2);
		gt_free(e);
		gt_free(u);
		gt_free(v);
		bn_free(k);
		bn_free(n);
		RLC_FREE(buf);
	}
	return result;
}