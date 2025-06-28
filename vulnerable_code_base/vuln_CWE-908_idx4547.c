int pico_tcp_initconn(struct pico_socket *s)
{
    struct pico_socket_tcp *ts = TCP_SOCK(s);
    struct pico_frame *syn;
    struct pico_tcp_hdr *hdr;
    uint16_t mtu, opt_len = tcp_options_size(ts, PICO_TCP_SYN);
    syn = s->net->alloc(s->stack, s->net, NULL, (uint16_t)(PICO_SIZE_TCPHDR + opt_len));
    if (!syn)
        return -1;
    hdr = (struct pico_tcp_hdr *) syn->transport_hdr;
    if (!ts->snd_nxt)
        ts->snd_nxt = long_be(pico_paws());
    ts->snd_last = ts->snd_nxt;
    ts->cwnd = PICO_TCP_IW;
    mtu = (uint16_t)pico_socket_get_mss(s);
    ts->mss = (uint16_t)(mtu - PICO_SIZE_TCPHDR);
    ts->ssthresh = (uint16_t)((uint16_t)(PICO_DEFAULT_SOCKETQ / ts->mss) -  (((uint16_t)(PICO_DEFAULT_SOCKETQ / ts->mss)) >> 3u));
    syn->sock = s;
    hdr->seq = long_be(ts->snd_nxt);
    hdr->len = (uint8_t)((PICO_SIZE_TCPHDR + opt_len) << 2);
    hdr->flags = PICO_TCP_SYN;
    tcp_set_space(ts);
    hdr->rwnd = short_be(ts->wnd);
    tcp_add_options(ts, syn, PICO_TCP_SYN, opt_len);
    hdr->trans.sport = ts->sock.local_port;
    hdr->trans.dport = ts->sock.remote_port;
    hdr->crc = 0;
    hdr->crc = short_be(pico_tcp_checksum(syn));
    tcp_dbg("Sending SYN... (ports: %d - %d) size: %d\n", short_be(ts->sock.local_port), short_be(ts->sock.remote_port), syn->buffer_len);
    ts->retrans_tmr = pico_timer_add(s->stack, PICO_TCP_SYN_TO << ts->backoff, initconn_retry, ts);
    if (!ts->retrans_tmr) {
        tcp_dbg("TCP: Failed to start initconn_retry timer\n");
        PICO_FREE(syn);
        return -1;
    }
    pico_enqueue(&s->stack->q_tcp.out, syn);
    return 0;
}