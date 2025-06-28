void gnrc_sixlowpan_frag_sfr_arq_timeout(gnrc_sixlowpan_frag_fb_t *fbuf)
{
    uint32_t now = xtimer_now_usec() / US_PER_MS;
    _frag_desc_t *frag_desc = (_frag_desc_t *)fbuf->sfr.window.next;
    uint32_t next_arq_offset = fbuf->sfr.arq_timeout;
    bool reschedule_arq_timeout = false;
    int error_no = ETIMEDOUT;    
    DEBUG("6lo sfr: ARQ timeout for datagram %u\n", fbuf->tag);
    fbuf->sfr.arq_timeout_event.msg.content.ptr = NULL;
    if (IS_ACTIVE(CONFIG_GNRC_SIXLOWPAN_SFR_MOCK_ARQ_TIMER)) {
        now -= (fbuf->sfr.arq_timeout * US_PER_MS) + 1;
    }
    if (IS_USED(MODULE_GNRC_SIXLOWPAN_FRAG_SFR_CONGURE) && frag_desc) {
        gnrc_sixlowpan_frag_sfr_congure_snd_report_frags_timeout(fbuf);
        _shrink_window(fbuf);    
        frag_desc = (_frag_desc_t *)fbuf->sfr.window.next;
    }
    _frag_desc_t * const head = frag_desc;
    if (frag_desc) {
        do {
            uint32_t diff;
            frag_desc = (_frag_desc_t *)frag_desc->super.super.next;
            diff = now - frag_desc->super.send_time;
            if (diff < fbuf->sfr.arq_timeout) {
                uint32_t offset = fbuf->sfr.arq_timeout - diff;
                DEBUG("6lo sfr: wait for fragment %u in next reschedule\n",
                      _frag_seq(frag_desc));
                if (offset < next_arq_offset) {
                    next_arq_offset = offset;
                    DEBUG("         (next ARQ timeout in %lu)\n",
                          (long unsigned)next_arq_offset);
                }
                reschedule_arq_timeout = true;
            }
            else if (_frag_ack_req(frag_desc)) {
                if ((frag_desc->super.resends++) < CONFIG_GNRC_SIXLOWPAN_SFR_FRAG_RETRIES) {
                    DEBUG("6lo sfr: %u retries left for fragment (tag: %u, "
                          "X: %i, seq: %u, frag_size: %u, offset: %u)\n",
                          CONFIG_GNRC_SIXLOWPAN_SFR_FRAG_RETRIES -
                          (frag_desc->super.resends - 1), (uint8_t)fbuf->tag,
                          _frag_ack_req(frag_desc), _frag_seq(frag_desc),
                          _frag_size(frag_desc), frag_desc->offset);
                    if (_resend_frag(&frag_desc->super.super, fbuf) != 0) {
                        error_no = ENOMEM;
                        goto error;
                    }
                    else {
                        if (IS_USED(MODULE_GNRC_SIXLOWPAN_FRAG_SFR_CONGURE)) {
                            gnrc_sixlowpan_frag_sfr_congure_snd_report_frag_sent(fbuf);
                        }
                        if (IS_USED(MODULE_GNRC_SIXLOWPAN_FRAG_SFR_STATS)) {
                            _stats.fragment_resends.by_timeout++;
                        }
                    }
                    reschedule_arq_timeout = true;
                }
                else {
                    DEBUG("6lo sfr: no retries left for fragment "
                          "(tag: %u, X: %i, seq: %u, frag_size: %u, "
                          "offset: %u)\n",
                          (uint8_t)fbuf->tag, _frag_ack_req(frag_desc),
                          _frag_seq(frag_desc), _frag_size(frag_desc),
                          frag_desc->offset);
                    _retry_datagram(fbuf);
                    return;
                }
            }
            else {
                DEBUG("6lo sfr: nothing to do for fragment %u\n",
                      _frag_seq(frag_desc));
            }
        } while (frag_desc != head);
        clist_foreach(&fbuf->sfr.window, _report_non_ack_req_window_sent, fbuf);
    }
    else {
        error_no = GNRC_NETERR_SUCCESS;
    }
    assert(fbuf->sfr.frags_sent == clist_count(&fbuf->sfr.window));
    if (reschedule_arq_timeout) {
        _sched_arq_timeout(fbuf, next_arq_offset);
        return;
    }
error:
    _send_abort_frag(fbuf->pkt, fbuf, false, 0);
    _clean_up_fbuf(fbuf, error_no);
}