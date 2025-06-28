void ep4_map(ep4_t p, const uint8_t *msg, int len) {
	bn_t x;
	fp4_t t0;
	uint8_t digest[RLC_MD_LEN];
	bn_null(x);
	fp4_null(t0);
	RLC_TRY {
		bn_new(x);
		fp4_new(t0);
		md_map(digest, msg, len);
		bn_read_bin(x, digest, RLC_MIN(RLC_FP_BYTES, RLC_MD_LEN));
		fp4_zero(p->x);
		fp_prime_conv(p->x[0][0], x);
		fp4_set_dig(p->z, 1);
		while (1) {
			ep4_rhs(t0, p);
			if (fp4_srt(p->y, t0)) {
				p->coord = BASIC;
				break;
			}
			fp_add_dig(p->x[0][0], p->x[0][0], 1);
		}
		ep4_mul_cof(p, p);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(x);
		fp4_free(t0);
	}
}