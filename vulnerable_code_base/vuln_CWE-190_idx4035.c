void ed_map_dst(ed_t p, const uint8_t *msg, int len, const uint8_t *dst, int dst_len) {
	bn_t k;
	fp_t t;
	ed_t q;
	const int len_per_elm = (FP_PRIME + ed_param_level() + 7) / 8;
	uint8_t *pseudo_random_bytes = RLC_ALLOCA(uint8_t, 2 * len_per_elm);
	bn_null(k);
	fp_null(t);
	ed_null(q);
	RLC_TRY {
		bn_new(k);
		fp_new(t);
		ed_new(q);
		md_xmd(pseudo_random_bytes, 2 * len_per_elm, msg, len, dst, dst_len);
#define ED_MAP_CONVERT_BYTES(IDX)                                                        \
	do {                                                                                 \
		bn_read_bin(k, pseudo_random_bytes + IDX * len_per_elm, len_per_elm);            \
		fp_prime_conv(t, k);                                                             \
	} while (0)
		ED_MAP_CONVERT_BYTES(0);
		ed_map_ell2_5mod8(p, t);
		ED_MAP_CONVERT_BYTES(1);
		ed_map_ell2_5mod8(q, t);
#undef ED_MAP_CONVERT_BYTES
		ed_add(p, p, q);
		switch (ed_param_get()) {
			case CURVE_ED25519:
				ed_dbl(p, p);
				ed_dbl(p, p);
				ed_dbl(p, p);
				break;
			default:
				RLC_THROW(ERR_NO_VALID);
				break;
		}
		ed_norm(p, p);
#if ED_ADD == EXTND
		fp_mul(p->t, p->x, p->y);
#endif
		p->coord = BASIC;
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(k);
		fp_free(t);
		ed_free(q);
		RLC_FREE(pseudo_random_bytes);
	}
}