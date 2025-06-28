static int tcp_syn(struct pico_socket *s, struct pico_frame *f)
{
    struct pico_socket_tcp *new = NULL;
    struct pico_tcp_hdr *hdr = NULL;
    uint16_t mtu;
    if(s->number_of_pending_conn >= s->max_backlog)
        return -1;
    new = (struct pico_socket_tcp *)pico_socket_clone(s);
    hdr = (struct pico_tcp_hdr *)f->transport_hdr;
    if (!new)
        return -1;
#ifdef PICO_TCP_SUPPORT_SOCKET_STATS
    if (!pico_timer_add(t->sock.stack, 2000, sock_stats, s)) {
        tcp_dbg("TCP: Failed to start socket statistics timer\n");
        return -1;
    }
#endif
    new->sock.remote_port = ((struct pico_trans *)f->transport_hdr)->sport;
#ifdef PICO_SUPPORT_IPV4
    if (IS_IPV4(f)) {
        new->sock.remote_addr.ip4.addr = ((struct pico_ipv4_hdr *)(f->net_hdr))->src.addr;
        new->sock.local_addr.ip4.addr = ((struct pico_ipv4_hdr *)(f->net_hdr))->dst.addr;
    }
#endif
#ifdef PICO_SUPPORT_IPV6
    if (IS_IPV6(f)) {
        new->sock.remote_addr.ip6 = ((struct pico_ipv6_hdr *)(f->net_hdr))->src;
        new->sock.local_addr.ip6 = ((struct pico_ipv6_hdr *)(f->net_hdr))->dst;
    }
#endif
    f->sock = &new->sock;
    mtu = (uint16_t)pico_socket_get_mss(&new->sock);
    new->mss = (uint16_t)(mtu - PICO_SIZE_TCPHDR);
    if (tcp_parse_options(f) < 0)
        return -1;
    new->sock.stack = s->stack;
    new->tcpq_in.max_size = PICO_DEFAULT_SOCKETQ;
    new->tcpq_out.max_size = PICO_DEFAULT_SOCKETQ;
    new->tcpq_hold.max_size = 2u * mtu;
    new->rcv_nxt = long_be(hdr->seq) + 1;
    new->snd_nxt = long_be(pico_paws());
    new->snd_last = new->snd_nxt;
    new->cwnd = PICO_TCP_IW;
    new->ssthresh = (uint16_t)((uint16_t)(PICO_DEFAULT_SOCKETQ / new->mss) -  (((uint16_t)(PICO_DEFAULT_SOCKETQ / new->mss)) >> 3u));
    new->recv_wnd = short_be(hdr->rwnd);
    new->linger_timeout = PICO_SOCKET_LINGER_TIMEOUT;
    s->number_of_pending_conn++;
    new->sock.parent = s;
    new->sock.wakeup = s->wakeup;
    rto_set(new, PICO_TCP_RTO_MIN);
    new->sock.state = PICO_SOCKET_STATE_BOUND | PICO_SOCKET_STATE_CONNECTED | PICO_SOCKET_STATE_TCP_SYN_RECV;
    pico_socket_add(&new->sock);
    tcp_send_synack(&new->sock);
    tcp_dbg("SYNACK sent, socket added. snd_nxt is %08x\n", new->snd_nxt);
    return 0;
}