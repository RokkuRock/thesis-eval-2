static int ax25_recvmsg(struct kiocb *iocb, struct socket *sock,
	struct msghdr *msg, size_t size, int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	int copied;
	int err = 0;
	lock_sock(sk);
	if (sk->sk_type == SOCK_SEQPACKET && sk->sk_state != TCP_ESTABLISHED) {
		err =  -ENOTCONN;
		goto out;
	}
	skb = skb_recv_datagram(sk, flags & ~MSG_DONTWAIT,
				flags & MSG_DONTWAIT, &err);
	if (skb == NULL)
		goto out;
	if (!ax25_sk(sk)->pidincl)
		skb_pull(skb, 1);		 
	skb_reset_transport_header(skb);
	copied = skb->len;
	if (copied > size) {
		copied = size;
		msg->msg_flags |= MSG_TRUNC;
	}
	skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);
	if (msg->msg_namelen != 0) {
		struct sockaddr_ax25 *sax = (struct sockaddr_ax25 *)msg->msg_name;
		ax25_digi digi;
		ax25_address src;
		const unsigned char *mac = skb_mac_header(skb);
		memset(sax, 0, sizeof(struct full_sockaddr_ax25));
		ax25_addr_parse(mac + 1, skb->data - mac - 1, &src, NULL,
				&digi, NULL, NULL);
		sax->sax25_family = AF_AX25;
		sax->sax25_ndigis = digi.ndigi;
		sax->sax25_call   = src;
		if (sax->sax25_ndigis != 0) {
			int ct;
			struct full_sockaddr_ax25 *fsa = (struct full_sockaddr_ax25 *)sax;
			for (ct = 0; ct < digi.ndigi; ct++)
				fsa->fsa_digipeater[ct] = digi.calls[ct];
		}
		msg->msg_namelen = sizeof(struct full_sockaddr_ax25);
	}
	skb_free_datagram(sk, skb);
	err = copied;
out:
	release_sock(sk);
	return err;
}