static size_t _iphc_ipv6_encode(gnrc_pktsnip_t *pkt,
                                const gnrc_netif_hdr_t *netif_hdr,
                                gnrc_netif_t *iface,
                                uint8_t *iphc_hdr)
{
    gnrc_sixlowpan_ctx_t *src_ctx = NULL, *dst_ctx = NULL;
    ipv6_hdr_t *ipv6_hdr = pkt->next->data;
    bool addr_comp = false;
    uint16_t inline_pos = SIXLOWPAN_IPHC_HDR_LEN;
    assert(iface != NULL);
    iphc_hdr[IPHC1_IDX] = SIXLOWPAN_IPHC1_DISP;
    iphc_hdr[IPHC2_IDX] = 0;
    if (!ipv6_addr_is_unspecified(&(ipv6_hdr->src))) {
        src_ctx = gnrc_sixlowpan_ctx_lookup_addr(&(ipv6_hdr->src));
        if (src_ctx && !(src_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_COMP)) {
            src_ctx = NULL;
        }
        if (src_ctx &&
            ipv6_addr_match_prefix(&src_ctx->prefix, &ipv6_hdr->src) < SIXLOWPAN_IPHC_PREFIX_LEN) {
            src_ctx = NULL;
        }
    }
    if (!ipv6_addr_is_multicast(&ipv6_hdr->dst)) {
        dst_ctx = gnrc_sixlowpan_ctx_lookup_addr(&(ipv6_hdr->dst));
        if (dst_ctx && !(dst_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_COMP)) {
            dst_ctx = NULL;
        }
        if (dst_ctx &&
            ipv6_addr_match_prefix(&dst_ctx->prefix, &ipv6_hdr->dst) < SIXLOWPAN_IPHC_PREFIX_LEN) {
            dst_ctx = NULL;
        }
    }
    if (((src_ctx != NULL) &&
            ((src_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) != 0)) ||
        ((dst_ctx != NULL) &&
            ((dst_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) != 0))) {
        iphc_hdr[IPHC2_IDX] |= SIXLOWPAN_IPHC2_CID_EXT;
        iphc_hdr[CID_EXT_IDX] = 0;
        inline_pos += SIXLOWPAN_IPHC_CID_EXT_LEN;
    }
    if (ipv6_hdr_get_fl(ipv6_hdr) == 0) {
        if (ipv6_hdr_get_tc(ipv6_hdr) == 0) {
            iphc_hdr[IPHC1_IDX] |= IPHC_TF_ECN_ELIDE;
        }
        else {
            iphc_hdr[IPHC1_IDX] |= IPHC_TF_ECN_DSCP;
            iphc_hdr[inline_pos++] = ipv6_hdr_get_tc(ipv6_hdr);
        }
    }
    else {
        if (ipv6_hdr_get_tc_dscp(ipv6_hdr) == 0) {
            iphc_hdr[IPHC1_IDX] |= IPHC_TF_ECN_FL;
            iphc_hdr[inline_pos++] = (uint8_t)((ipv6_hdr_get_tc_ecn(ipv6_hdr) << 6) |
                                               ((ipv6_hdr_get_fl(ipv6_hdr) & 0x000f0000) >> 16));
        }
        else {
            iphc_hdr[IPHC1_IDX] |= IPHC_TF_ECN_DSCP_FL;
            iphc_hdr[inline_pos++] = ipv6_hdr_get_tc(ipv6_hdr);
            iphc_hdr[inline_pos++] = (uint8_t)((ipv6_hdr_get_fl(ipv6_hdr) & 0x000f0000) >> 16);
        }
        iphc_hdr[inline_pos++] = (uint8_t)((ipv6_hdr_get_fl(ipv6_hdr) & 0x0000ff00) >> 8);
        iphc_hdr[inline_pos++] = (uint8_t)(ipv6_hdr_get_fl(ipv6_hdr) & 0x000000ff);
    }
    if (_compressible_nh(ipv6_hdr->nh)) {
        iphc_hdr[IPHC1_IDX] |= SIXLOWPAN_IPHC1_NH;
    }
    else {
        iphc_hdr[inline_pos++] = ipv6_hdr->nh;
    }
    switch (ipv6_hdr->hl) {
        case 1:
            iphc_hdr[IPHC1_IDX] |= IPHC_HL_1;
            break;
        case 64:
            iphc_hdr[IPHC1_IDX] |= IPHC_HL_64;
            break;
        case 255:
            iphc_hdr[IPHC1_IDX] |= IPHC_HL_255;
            break;
        default:
            iphc_hdr[IPHC1_IDX] |= IPHC_HL_INLINE;
            iphc_hdr[inline_pos++] = ipv6_hdr->hl;
            break;
    }
    if (ipv6_addr_is_unspecified(&(ipv6_hdr->src))) {
        iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_UNSPEC;
    }
    else {
        if (src_ctx != NULL) {
            iphc_hdr[IPHC2_IDX] |= SIXLOWPAN_IPHC2_SAC;
            if (((src_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) != 0)) {
                iphc_hdr[CID_EXT_IDX] |= ((src_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) << 4);
            }
        }
        if ((src_ctx != NULL) || ipv6_addr_is_link_local(&(ipv6_hdr->src))) {
            eui64_t iid;
            iid.uint64.u64 = 0;
            gnrc_netif_acquire(iface);
            if (gnrc_netif_ipv6_get_iid(iface, &iid) < 0) {
                DEBUG("6lo iphc: could not get interface's IID\n");
                gnrc_netif_release(iface);
                return 0;
            }
            gnrc_netif_release(iface);
            if ((ipv6_hdr->src.u64[1].u64 == iid.uint64.u64) ||
                _context_overlaps_iid(src_ctx, &ipv6_hdr->src, &iid)) {
                iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_L2;
                addr_comp = true;
            }
            else if ((byteorder_ntohl(ipv6_hdr->src.u32[2]) == 0x000000ff) &&
                     (byteorder_ntohs(ipv6_hdr->src.u16[6]) == 0xfe00)) {
                iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_16;
                memcpy(iphc_hdr + inline_pos, ipv6_hdr->src.u16 + 7, 2);
                inline_pos += 2;
                addr_comp = true;
            }
            else {
                iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_64;
                memcpy(iphc_hdr + inline_pos, ipv6_hdr->src.u64 + 1, 8);
                inline_pos += 8;
                addr_comp = true;
            }
        }
        if (!addr_comp) {
            iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_FULL;
            memcpy(iphc_hdr + inline_pos, &ipv6_hdr->src, 16);
            inline_pos += 16;
        }
    }
    addr_comp = false;
    if (ipv6_addr_is_multicast(&(ipv6_hdr->dst))) {
        iphc_hdr[IPHC2_IDX] |= SIXLOWPAN_IPHC2_M;
        if ((ipv6_hdr->dst.u16[1].u16 == 0) &&
            (ipv6_hdr->dst.u32[1].u32 == 0) &&
            (ipv6_hdr->dst.u16[4].u16 == 0)) {
            if ((ipv6_hdr->dst.u8[1] == 0x02) &&
                (ipv6_hdr->dst.u32[2].u32 == 0) &&
                (ipv6_hdr->dst.u16[6].u16 == 0) &&
                (ipv6_hdr->dst.u8[14] == 0)) {
                iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_M_8;
                iphc_hdr[inline_pos++] = ipv6_hdr->dst.u8[15];
                addr_comp = true;
            }
            else if ((ipv6_hdr->dst.u16[5].u16 == 0) &&
                     (ipv6_hdr->dst.u8[12] == 0)) {
                iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_M_32;
                iphc_hdr[inline_pos++] = ipv6_hdr->dst.u8[1];
                memcpy(iphc_hdr + inline_pos, ipv6_hdr->dst.u8 + 13, 3);
                inline_pos += 3;
                addr_comp = true;
            }
            else if (ipv6_hdr->dst.u8[10] == 0) {
                iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_M_48;
                iphc_hdr[inline_pos++] = ipv6_hdr->dst.u8[1];
                memcpy(iphc_hdr + inline_pos, ipv6_hdr->dst.u8 + 11, 5);
                inline_pos += 5;
                addr_comp = true;
            }
        }
        else {
            gnrc_sixlowpan_ctx_t *ctx;
            ipv6_addr_t unicast_prefix;
            unicast_prefix.u16[0] = ipv6_hdr->dst.u16[2];
            unicast_prefix.u16[1] = ipv6_hdr->dst.u16[3];
            unicast_prefix.u16[2] = ipv6_hdr->dst.u16[4];
            unicast_prefix.u16[3] = ipv6_hdr->dst.u16[5];
            ctx = gnrc_sixlowpan_ctx_lookup_addr(&unicast_prefix);
            if ((ctx != NULL) && (ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_COMP) &&
                (ctx->prefix_len == ipv6_hdr->dst.u8[3])) {
                iphc_hdr[IPHC2_IDX] |= SIXLOWPAN_IPHC2_DAC;
                if ((ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) != 0) {
                    iphc_hdr[CID_EXT_IDX] |= (ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK);
                }
                iphc_hdr[inline_pos++] = ipv6_hdr->dst.u8[1];
                iphc_hdr[inline_pos++] = ipv6_hdr->dst.u8[2];
                memcpy(iphc_hdr + inline_pos, ipv6_hdr->dst.u16 + 6, 4);
                inline_pos += 4;
                addr_comp = true;
            }
        }
    }
    else if (((dst_ctx != NULL) ||
              ipv6_addr_is_link_local(&ipv6_hdr->dst)) && (netif_hdr->dst_l2addr_len > 0)) {
        eui64_t iid;
        if (dst_ctx != NULL) {
            iphc_hdr[IPHC2_IDX] |= SIXLOWPAN_IPHC2_DAC;
            if (((dst_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK) != 0)) {
                iphc_hdr[CID_EXT_IDX] |= (dst_ctx->flags_id & GNRC_SIXLOWPAN_CTX_FLAGS_CID_MASK);
            }
        }
        if (gnrc_netif_hdr_ipv6_iid_from_dst(iface, netif_hdr, &iid) < 0) {
            DEBUG("6lo iphc: could not get destination's IID\n");
            return 0;
        }
        if ((ipv6_hdr->dst.u64[1].u64 == iid.uint64.u64) ||
            _context_overlaps_iid(dst_ctx, &(ipv6_hdr->dst), &iid)) {
            iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_U_L2;
            addr_comp = true;
        }
        else if ((byteorder_ntohl(ipv6_hdr->dst.u32[2]) == 0x000000ff) &&
                 (byteorder_ntohs(ipv6_hdr->dst.u16[6]) == 0xfe00)) {
            iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_U_16;
            memcpy(&(iphc_hdr[inline_pos]), &(ipv6_hdr->dst.u16[7]), 2);
            inline_pos += 2;
            addr_comp = true;
        }
        else {
            iphc_hdr[IPHC2_IDX] |= IPHC_M_DAC_DAM_U_64;
            memcpy(&(iphc_hdr[inline_pos]), &(ipv6_hdr->dst.u8[8]), 8);
            inline_pos += 8;
            addr_comp = true;
        }
    }
    if (!addr_comp) {
        iphc_hdr[IPHC2_IDX] |= IPHC_SAC_SAM_FULL;
        memcpy(iphc_hdr + inline_pos, &ipv6_hdr->dst, 16);
        inline_pos += 16;
    }
    return inline_pos;
}