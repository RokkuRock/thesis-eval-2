void ep2_map_from_field(ep2_t p, const uint8_t *uniform_bytes, int len) {
        bn_t k;
        fp2_t t;
        ep2_t q;
        int neg;
        const int len_per_elm = (FP_PRIME + ep_param_level() + 7) / 8;
        bn_null(k);
        fp2_null(t);
        ep2_null(q);
        RLC_TRY {
                if (len != 2* len_per_elm) {
                  RLC_THROW(ERR_NO_VALID);
                }
                bn_new(k);
                fp2_new(t);
                ep2_new(q);
                const int abNeq0 = (ep2_curve_opt_a() != RLC_ZERO) && (ep2_curve_opt_b() != RLC_ZERO);
                void (*const map_fn)(ep2_t, fp2_t) = (ep2_curve_is_ctmap() || abNeq0) ? ep2_map_sswu : ep2_map_svdw;
#define EP2_MAP_CONVERT_BYTES(IDX)                                                       \
        do {                                                                                 \
                bn_read_bin(k, uniform_bytes + 2 * IDX * len_per_elm, len_per_elm);        \
                fp_prime_conv(t[0], k);                                                          \
                bn_read_bin(k, uniform_bytes + (2 * IDX + 1) * len_per_elm, len_per_elm);  \
                fp_prime_conv(t[1], k);                                                          \
        } while (0)
#define EP2_MAP_APPLY_MAP(PT)                                                            \
        do {                                                                                 \
                                                                                   \
                neg = fp2_sgn0(t, k);                                                            \
                                                                                     \
                map_fn(PT, t);                                                                   \
                                            \
                neg = neg != fp2_sgn0(PT->y, k);                                                 \
                fp2_neg(t, PT->y);                                                               \
                dv_copy_cond(PT->y[0], t[0], RLC_FP_DIGS, neg);                                  \
                dv_copy_cond(PT->y[1], t[1], RLC_FP_DIGS, neg);                                  \
        } while (0)
                EP2_MAP_CONVERT_BYTES(0);
                EP2_MAP_APPLY_MAP(p);
                TMPL_MAP_CALL_ISOMAP(ep2, p);
                EP2_MAP_CONVERT_BYTES(1);
                EP2_MAP_APPLY_MAP(q);
                TMPL_MAP_CALL_ISOMAP(ep2, q);
#undef EP2_MAP_CONVERT_BYTES
#undef EP2_MAP_APPLY_MAP
                ep2_add(p, p, q);
                ep2_norm(p, p);
                ep2_mul_cof(p, p);
        }
        RLC_CATCH_ANY {
                RLC_THROW(ERR_CAUGHT);
        }
        RLC_FINALLY {
                bn_free(k);
                fp2_free(t);
                ep2_free(q);
        }
}