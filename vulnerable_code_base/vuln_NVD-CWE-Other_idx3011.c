static unsigned int unix_dgram_poll(struct file *file, struct socket *sock,
				    poll_table *wait)
{
	struct sock *sk = sock->sk, *other;
	unsigned int mask, writable;
	sock_poll_wait(file, sk_sleep(sk), wait);
	mask = 0;
	if (sk->sk_err || !skb_queue_empty(&sk->sk_error_queue))
		mask |= POLLERR |
			(sock_flag(sk, SOCK_SELECT_ERR_QUEUE) ? POLLPRI : 0);
	if (sk->sk_shutdown & RCV_SHUTDOWN)
		mask |= POLLRDHUP | POLLIN | POLLRDNORM;
	if (sk->sk_shutdown == SHUTDOWN_MASK)
		mask |= POLLHUP;
	if (!skb_queue_empty(&sk->sk_receive_queue))
		mask |= POLLIN | POLLRDNORM;
	if (sk->sk_type == SOCK_SEQPACKET) {
		if (sk->sk_state == TCP_CLOSE)
			mask |= POLLHUP;
		if (sk->sk_state == TCP_SYN_SENT)
			return mask;
	}
	if (!(poll_requested_events(wait) & (POLLWRBAND|POLLWRNORM|POLLOUT)))
		return mask;
	writable = unix_writable(sk);
	other = unix_peer_get(sk);
	if (other) {
		if (unix_peer(other) != sk) {
			sock_poll_wait(file, &unix_sk(other)->peer_wait, wait);
			if (unix_recvq_full(other))
				writable = 0;
		}
		sock_put(other);
	}
	if (writable)
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;
	else
		set_bit(SOCK_ASYNC_NOSPACE, &sk->sk_socket->flags);
	return mask;
}