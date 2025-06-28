int cp_cmlhs_gen(bn_t x[], gt_t hs[], size_t len, uint8_t prf[], size_t plen,
		bn_t sk, g2_t pk, bn_t d, g2_t y, int bls) {
	g1_t g1;
	g2_t g2;
	gt_t gt;
	bn_t n;
	int result = RLC_OK;
	g1_null(g1);
	g2_null(g2);
	gt_null(gt);
	bn_null(n);
	RLC_TRY {
		bn_new(n);
		g1_new(g1);
		g2_new(g2);
		gt_new(gt);
		pc_get_ord(n);
		g1_get_gen(g1);
		g2_get_gen(g2);
		pc_map(gt, g1, g2);
		rand_bytes(prf, plen);
		if (bls) {
			cp_bls_gen(sk, pk);
		} else {
			cp_ecdsa_gen(sk, g1);
			fp_copy(pk->x[0], g1->x);
			fp_copy(pk->y[0], g1->y);
		}
		for (int i = 0; i < len; i++) {
			bn_rand_mod(x[i], n);
			gt_exp(hs[i], gt, x[i]);
		}
		bn_rand_mod(d, n);
		g2_mul_gen(y, d);
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		g1_free(g1);
		g2_free(g2);
		gt_free(gt);
		bn_free(n);
	}
	return result;
}