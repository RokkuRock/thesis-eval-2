struct pico_socket *pico_tcp_open(struct pico_stack *S, uint16_t family)
{
    struct pico_socket_tcp *t = PICO_ZALLOC(sizeof(struct pico_socket_tcp));
    if (!t)
        return NULL;
    t->sock.stack = S;
    t->sock.timestamp = TCP_TIME;
    pico_socket_set_family(&t->sock, family);
    t->mss = (uint16_t)(pico_socket_get_mss(&t->sock) - PICO_SIZE_TCPHDR);
    t->tcpq_in.pool.root = t->tcpq_hold.pool.root = t->tcpq_out.pool.root = &LEAF;
    t->tcpq_hold.pool.compare = t->tcpq_out.pool.compare = segment_compare;
    t->tcpq_in.pool.compare = input_segment_compare;
    t->tcpq_in.max_size = PICO_DEFAULT_SOCKETQ;
    t->tcpq_out.max_size = PICO_DEFAULT_SOCKETQ;
    t->tcpq_hold.max_size = 2u * t->mss;
    rto_set(t, PICO_TCP_RTO_MIN);
    t->sock.opt_flags |= (1 << PICO_SOCKET_OPT_TCPNODELAY);
    t->linger_timeout = PICO_SOCKET_LINGER_TIMEOUT;
#ifdef PICO_TCP_SUPPORT_SOCKET_STATS
    if (!pico_timer_add(t->sock.stack, 2000, sock_stats, t)) {
        tcp_dbg("TCP: Failed to start socket statistics timer\n");
        PICO_FREE(t);
        return NULL;
    }
#endif
    t->keepalive_tmr = pico_timer_add(t->sock.stack, 1000, pico_tcp_keepalive, t);
    if (!t->keepalive_tmr) {
        tcp_dbg("TCP: Failed to start keepalive timer\n");
        PICO_FREE(t);
        return NULL;
    }
    tcp_set_space(t);
    return &t->sock;
}