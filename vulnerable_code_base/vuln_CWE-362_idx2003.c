static void _sched_arq_timeout(gnrc_sixlowpan_frag_fb_t *fbuf, uint32_t offset)
{
    if (IS_ACTIVE(CONFIG_GNRC_SIXLOWPAN_SFR_MOCK_ARQ_TIMER)) {
        return;
    }
    if (fbuf->sfr.arq_timeout_event.msg.content.ptr != NULL) {
        DEBUG("6lo sfr: ARQ timeout for datagram %u already scheduled\n",
              (uint8_t)fbuf->tag);
        return;
    }
    DEBUG("6lo sfr: arming ACK timeout in %lums for datagram %u\n",
          (long unsigned)offset, fbuf->tag);
    fbuf->sfr.arq_timeout_event.event.offset = offset;
    fbuf->sfr.arq_timeout_event.msg.content.ptr = fbuf;
    fbuf->sfr.arq_timeout_event.msg.type = GNRC_SIXLOWPAN_FRAG_SFR_ARQ_TIMEOUT_MSG;
    evtimer_add_msg(&_arq_timer, &fbuf->sfr.arq_timeout_event,
                    _getpid());
}