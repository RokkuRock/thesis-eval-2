static int recv_msg(struct kiocb *iocb, struct socket *sock,
		    struct msghdr *m, size_t buf_len, int flags)
{
	struct sock *sk = sock->sk;
	struct tipc_port *tport = tipc_sk_port(sk);
	struct sk_buff *buf;
	struct tipc_msg *msg;
	long timeout;
	unsigned int sz;
	u32 err;
	int res;
	if (unlikely(!buf_len))
		return -EINVAL;
	lock_sock(sk);
	if (unlikely(sock->state == SS_UNCONNECTED)) {
		res = -ENOTCONN;
		goto exit;
	}
	m->msg_namelen = 0;
	timeout = sock_rcvtimeo(sk, flags & MSG_DONTWAIT);
restart:
	while (skb_queue_empty(&sk->sk_receive_queue)) {
		if (sock->state == SS_DISCONNECTING) {
			res = -ENOTCONN;
			goto exit;
		}
		if (timeout <= 0L) {
			res = timeout ? timeout : -EWOULDBLOCK;
			goto exit;
		}
		release_sock(sk);
		timeout = wait_event_interruptible_timeout(*sk_sleep(sk),
							   tipc_rx_ready(sock),
							   timeout);
		lock_sock(sk);
	}
	buf = skb_peek(&sk->sk_receive_queue);
	msg = buf_msg(buf);
	sz = msg_data_sz(msg);
	err = msg_errcode(msg);
	if ((!sz) && (!err)) {
		advance_rx_queue(sk);
		goto restart;
	}
	set_orig_addr(m, msg);
	res = anc_data_recv(m, msg, tport);
	if (res)
		goto exit;
	if (!err) {
		if (unlikely(buf_len < sz)) {
			sz = buf_len;
			m->msg_flags |= MSG_TRUNC;
		}
		res = skb_copy_datagram_iovec(buf, msg_hdr_sz(msg),
					      m->msg_iov, sz);
		if (res)
			goto exit;
		res = sz;
	} else {
		if ((sock->state == SS_READY) ||
		    ((err == TIPC_CONN_SHUTDOWN) || m->msg_control))
			res = 0;
		else
			res = -ECONNRESET;
	}
	if (likely(!(flags & MSG_PEEK))) {
		if ((sock->state != SS_READY) &&
		    (++tport->conn_unacked >= TIPC_FLOW_CONTROL_WIN))
			tipc_acknowledge(tport->ref, tport->conn_unacked);
		advance_rx_queue(sk);
	}
exit:
	release_sock(sk);
	return res;
}