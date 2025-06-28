void ep_map_from_field(ep_t p, const uint8_t *uniform_bytes, int len) {
	bn_t k;
	fp_t t;
	ep_t q;
	int neg;
	const int len_per_elm = (FP_PRIME + ep_param_level() + 7) / 8;
	bn_null(k);
	fp_null(t);
	ep_null(q);
	RLC_TRY {
		if (len != 2 * len_per_elm) {
			RLC_THROW(ERR_NO_VALID);
		}
		bn_new(k);
		fp_new(t);
		ep_new(q);
		const int abNeq0 = (ep_curve_opt_a() != RLC_ZERO) &&
				(ep_curve_opt_b() != RLC_ZERO);
		void (*const map_fn)(ep_t, fp_t) =(ep_curve_is_ctmap() ||
				abNeq0) ? ep_map_sswu : ep_map_svdw;
#define EP_MAP_CONVERT_BYTES(IDX)                                       \
    do {                                                                \
      bn_read_bin(k, uniform_bytes + IDX * len_per_elm, len_per_elm);   \
      fp_prime_conv(t, k);                                              \
    } while (0)
#define EP_MAP_APPLY_MAP(PT)                                    \
    do {                                                        \
                                            \
      neg = fp_sgn0(t, k);                                      \
                                                    \
      map_fn(PT, t);                                            \
          \
      neg = neg != fp_sgn0(PT->y, k);                             \
      fp_neg(t, PT->y);                                          \
      dv_copy_cond(PT->y, t, RLC_FP_DIGS, neg);                  \
    } while (0)
		EP_MAP_CONVERT_BYTES(0);
		EP_MAP_APPLY_MAP(p);
		TMPL_MAP_CALL_ISOMAP(ep, p);
		EP_MAP_CONVERT_BYTES(1);
		EP_MAP_APPLY_MAP(q);
		TMPL_MAP_CALL_ISOMAP(ep, q);
#undef EP_MAP_CONVERT_BYTES
#undef EP_MAP_APPLY_MAP
		ep_add(p, p, q);
		ep_norm(p, p);
		switch (ep_curve_is_pairf()) {
			case EP_BN:
				break;
			case EP_B12:
			case EP_B24:
				fp_prime_get_par(k);
				bn_neg(k, k);
				bn_add_dig(k, k, 1);
				if (bn_bits(k) < RLC_DIG) {
					ep_mul_dig(p, p, k->dp[0]);
				} else {
					ep_mul(p, p, k);
				}
				break;
			default:
				ep_curve_get_cof(k);
				if (bn_bits(k) < RLC_DIG) {
					ep_mul_dig(p, p, k->dp[0]);
				} else {
					ep_mul_basic(p, p, k);
				}
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(k);
		fp_free(t);
		ep_free(q);
	}
}