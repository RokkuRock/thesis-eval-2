void ep2_map_dst(ep2_t p, const uint8_t *msg, int len, const uint8_t *dst, int dst_len) {
        const int len_per_elm = (FP_PRIME + ep_param_level() + 7) / 8;
        uint8_t *pseudo_random_bytes = RLC_ALLOCA(uint8_t, 4 * len_per_elm);
        RLC_TRY {
                md_xmd(pseudo_random_bytes, 4 * len_per_elm, msg, len, dst, dst_len);
                ep2_map_from_field(p, pseudo_random_bytes, 2 * len_per_elm);
        }
        RLC_CATCH_ANY {
                RLC_THROW(ERR_CAUGHT);
        }
        RLC_FINALLY {
                RLC_FREE(pseudo_random_bytes);
        }
}