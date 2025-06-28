void eb_map(eb_t p, const uint8_t *msg, int len) {
	bn_t k;
	fb_t t0, t1;
	int i;
	uint8_t digest[RLC_MD_LEN];
	bn_null(k);
	fb_null(t0);
	fb_null(t1);
	RLC_TRY {
		bn_new(k);
		fb_new(t0);
		fb_new(t1);
		md_map(digest, msg, len);
		bn_read_bin(k, digest, RLC_MIN(RLC_FB_BYTES, RLC_MD_LEN));
		fb_set_dig(p->z, 1);
		i = 0;
		while (1) {
			bn_add_dig(k, k, 1);
			bn_mod_2b(k, k, RLC_FB_BITS);
			dv_copy(p->x, k->dp, RLC_FB_DIGS);
			eb_rhs(t1, p);
			fb_sqr(t0, p->x);
			fb_inv(t0, t0);
			fb_mul(t0, t0, t1);
			if (fb_trc(t0) != 0) {
				i++;
			} else {
				fb_slv(t1, t0);
				fb_mul(p->y, t1, p->x);
				fb_set_dig(p->z, 1);
				p->coord = BASIC;
				break;
			}
		}
		eb_curve_get_cof(k);
		if (bn_bits(k) < RLC_DIG) {
			eb_mul_dig(p, p, k->dp[0]);
		} else {
			eb_mul(p, p, k);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(k);
		fb_free(t0);
		fb_free(t1);
	}
}