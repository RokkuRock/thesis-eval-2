static void _clean_slate_datagram(gnrc_sixlowpan_frag_fb_t *fbuf)
{
    clist_node_t new_queue = { .next = NULL };
    fbuf->sfr.arq_timeout_event.msg.content.ptr = NULL;
    evtimer_del((evtimer_t *)(&_arq_timer),
                &fbuf->sfr.arq_timeout_event.event);
    fbuf->sfr.arq_timeout_event.event.next = NULL;
    if (gnrc_sixlowpan_frag_sfr_congure_snd_has_inter_frame_gap()) {
        for (clist_node_t *node = clist_lpop(&_frame_queue);
             node != NULL; node = clist_lpop(&_frame_queue)) {
            _frame_queue_t *entry = (_frame_queue_t *)node;
            if (entry->datagram_tag == fbuf->tag) {
                gnrc_pktbuf_release(entry->frame);
                entry->frame = NULL;
                clist_rpush(&_frag_descs_free, node);
            }
            else {
                clist_rpush(&new_queue, node);
            }
        }
        _frame_queue = new_queue;
    }
    fbuf->offset = 0U;
    fbuf->sfr.cur_seq = 0U;
    fbuf->sfr.frags_sent = 0U;
    for (clist_node_t *node = clist_lpop(&fbuf->sfr.window);
         node != NULL; node = clist_lpop(&fbuf->sfr.window)) {
        clist_rpush(&_frag_descs_free, node);
    }
}