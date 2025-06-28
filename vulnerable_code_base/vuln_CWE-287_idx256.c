static int unix_stream_sendmsg(struct kiocb *kiocb, struct socket *sock,
			       struct msghdr *msg, size_t len)
{
	struct sock_iocb *siocb = kiocb_to_siocb(kiocb);
	struct sock *sk = sock->sk;
	struct sock *other = NULL;
	int err, size;
	struct sk_buff *skb;
	int sent = 0;
	struct scm_cookie tmp_scm;
	bool fds_sent = false;
	int max_level;
	if (NULL == siocb->scm)
		siocb->scm = &tmp_scm;
	wait_for_unix_gc();
	err = scm_send(sock, msg, siocb->scm);
	if (err < 0)
		return err;
	err = -EOPNOTSUPP;
	if (msg->msg_flags&MSG_OOB)
		goto out_err;
	if (msg->msg_namelen) {
		err = sk->sk_state == TCP_ESTABLISHED ? -EISCONN : -EOPNOTSUPP;
		goto out_err;
	} else {
		err = -ENOTCONN;
		other = unix_peer(sk);
		if (!other)
			goto out_err;
	}
	if (sk->sk_shutdown & SEND_SHUTDOWN)
		goto pipe_err;
	while (sent < len) {
		size = len-sent;
		if (size > ((sk->sk_sndbuf >> 1) - 64))
			size = (sk->sk_sndbuf >> 1) - 64;
		if (size > SKB_MAX_ALLOC)
			size = SKB_MAX_ALLOC;
		skb = sock_alloc_send_skb(sk, size, msg->msg_flags&MSG_DONTWAIT,
					  &err);
		if (skb == NULL)
			goto out_err;
		size = min_t(int, size, skb_tailroom(skb));
		err = unix_scm_to_skb(siocb->scm, skb, !fds_sent);
		if (err < 0) {
			kfree_skb(skb);
			goto out_err;
		}
		max_level = err + 1;
		fds_sent = true;
		err = memcpy_fromiovec(skb_put(skb, size), msg->msg_iov, size);
		if (err) {
			kfree_skb(skb);
			goto out_err;
		}
		unix_state_lock(other);
		if (sock_flag(other, SOCK_DEAD) ||
		    (other->sk_shutdown & RCV_SHUTDOWN))
			goto pipe_err_free;
		maybe_add_creds(skb, sock, other);
		skb_queue_tail(&other->sk_receive_queue, skb);
		if (max_level > unix_sk(other)->recursion_level)
			unix_sk(other)->recursion_level = max_level;
		unix_state_unlock(other);
		other->sk_data_ready(other, size);
		sent += size;
	}
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent;
pipe_err_free:
	unix_state_unlock(other);
	kfree_skb(skb);
pipe_err:
	if (sent == 0 && !(msg->msg_flags&MSG_NOSIGNAL))
		send_sig(SIGPIPE, current, 0);
	err = -EPIPE;
out_err:
	scm_destroy(siocb->scm);
	siocb->scm = NULL;
	return sent ? : err;
}