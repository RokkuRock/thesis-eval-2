static void _handle_ack(gnrc_netif_hdr_t *netif_hdr, gnrc_pktsnip_t *pkt,
                        unsigned page)
{
    gnrc_sixlowpan_frag_vrb_t *vrbe;
    sixlowpan_sfr_ack_t *hdr = pkt->data;
    uint32_t recv_time = xtimer_now_usec();
    (void)page;
    DEBUG("6lo sfr: received ACK for datagram (%s, %02x): %02X%02X%02X%02X\n",
          gnrc_netif_addr_to_str(gnrc_netif_hdr_get_src_addr(netif_hdr),
                                 netif_hdr->src_l2addr_len,
                                 addr_str), hdr->base.tag,
          hdr->bitmap[0], hdr->bitmap[1], hdr->bitmap[2], hdr->bitmap[3]);
    if ((vrbe = gnrc_sixlowpan_frag_vrb_reverse(
            gnrc_netif_hdr_get_netif(netif_hdr),
            gnrc_netif_hdr_get_src_addr(netif_hdr),
            netif_hdr->src_l2addr_len, hdr->base.tag)) != NULL) {
        sixlowpan_sfr_t mock_base = { .disp_ecn = hdr->base.disp_ecn,
                                      .tag = vrbe->super.tag };
        DEBUG("6lo sfr: forward ACK to (%s, %02x)\n",
              gnrc_netif_addr_to_str(vrbe->super.src, vrbe->super.src_len,
                                     addr_str), vrbe->super.tag);
        _send_ack(vrbe->in_netif, vrbe->super.src, vrbe->super.src_len,
                  &mock_base, hdr->bitmap);
        if (IS_USED(MODULE_GNRC_SIXLOWPAN_FRAG_SFR_STATS)) {
            _stats.acks.forwarded++;
        }
        if ((unaligned_get_u32(hdr->bitmap) == _full_bitmap.u32) ||
            (unaligned_get_u32(hdr->bitmap) == _null_bitmap.u32)) {
            if (CONFIG_GNRC_SIXLOWPAN_FRAG_RBUF_DEL_TIMER > 0) {
                vrbe->super.arrival = recv_time -
                                      (CONFIG_GNRC_SIXLOWPAN_FRAG_VRB_TIMEOUT_US -
                                       CONFIG_GNRC_SIXLOWPAN_FRAG_RBUF_DEL_TIMER);
            }
            else {
                gnrc_sixlowpan_frag_vrb_rm(vrbe);
            }
        }
        else {
            vrbe->super.arrival = recv_time;
        }
    }
    else {
        gnrc_sixlowpan_frag_fb_t *fbuf;
        if ((fbuf = gnrc_sixlowpan_frag_fb_get_by_tag(hdr->base.tag)) != NULL) {
            DEBUG("6lo sfr: cancelling ARQ timeout\n");
            evtimer_del((evtimer_t *)(&_arq_timer),
                        &fbuf->sfr.arq_timeout_event.event);
            fbuf->sfr.arq_timeout_event.msg.content.ptr = NULL;
            if ((unaligned_get_u32(hdr->bitmap) == _null_bitmap.u32)) {
                DEBUG("6lo sfr: fragmentation canceled\n");
                _retry_datagram(fbuf);
            }
            else {
               _check_failed_frags(hdr, fbuf, recv_time / US_PER_MS);
            }
        }
        else {
            DEBUG("6lo sfr: no VRB or fragmentation buffer found\n");
        }
    }
    gnrc_pktbuf_release(pkt);
}