vsock_stream_recvmsg(struct kiocb *kiocb,
		     struct socket *sock,
		     struct msghdr *msg, size_t len, int flags)
{
	struct sock *sk;
	struct vsock_sock *vsk;
	int err;
	size_t target;
	ssize_t copied;
	long timeout;
	struct vsock_transport_recv_notify_data recv_data;
	DEFINE_WAIT(wait);
	sk = sock->sk;
	vsk = vsock_sk(sk);
	err = 0;
	msg->msg_namelen = 0;
	lock_sock(sk);
	if (sk->sk_state != SS_CONNECTED) {
		if (sock_flag(sk, SOCK_DONE))
			err = 0;
		else
			err = -ENOTCONN;
		goto out;
	}
	if (flags & MSG_OOB) {
		err = -EOPNOTSUPP;
		goto out;
	}
	if (sk->sk_shutdown & RCV_SHUTDOWN) {
		err = 0;
		goto out;
	}
	if (!len) {
		err = 0;
		goto out;
	}
	target = sock_rcvlowat(sk, flags & MSG_WAITALL, len);
	if (target >= transport->stream_rcvhiwat(vsk)) {
		err = -ENOMEM;
		goto out;
	}
	timeout = sock_rcvtimeo(sk, flags & MSG_DONTWAIT);
	copied = 0;
	err = transport->notify_recv_init(vsk, target, &recv_data);
	if (err < 0)
		goto out;
	prepare_to_wait(sk_sleep(sk), &wait, TASK_INTERRUPTIBLE);
	while (1) {
		s64 ready = vsock_stream_has_data(vsk);
		if (ready < 0) {
			err = -ENOMEM;
			goto out_wait;
		} else if (ready > 0) {
			ssize_t read;
			err = transport->notify_recv_pre_dequeue(
					vsk, target, &recv_data);
			if (err < 0)
				break;
			read = transport->stream_dequeue(
					vsk, msg->msg_iov,
					len - copied, flags);
			if (read < 0) {
				err = -ENOMEM;
				break;
			}
			copied += read;
			err = transport->notify_recv_post_dequeue(
					vsk, target, read,
					!(flags & MSG_PEEK), &recv_data);
			if (err < 0)
				goto out_wait;
			if (read >= target || flags & MSG_PEEK)
				break;
			target -= read;
		} else {
			if (sk->sk_err != 0 || (sk->sk_shutdown & RCV_SHUTDOWN)
			    || (vsk->peer_shutdown & SEND_SHUTDOWN)) {
				break;
			}
			if (timeout == 0) {
				err = -EAGAIN;
				break;
			}
			err = transport->notify_recv_pre_block(
					vsk, target, &recv_data);
			if (err < 0)
				break;
			release_sock(sk);
			timeout = schedule_timeout(timeout);
			lock_sock(sk);
			if (signal_pending(current)) {
				err = sock_intr_errno(timeout);
				break;
			} else if (timeout == 0) {
				err = -EAGAIN;
				break;
			}
			prepare_to_wait(sk_sleep(sk), &wait,
					TASK_INTERRUPTIBLE);
		}
	}
	if (sk->sk_err)
		err = -sk->sk_err;
	else if (sk->sk_shutdown & RCV_SHUTDOWN)
		err = 0;
	if (copied > 0) {
		if (!(flags & MSG_PEEK)) {
			if (vsk->peer_shutdown & SEND_SHUTDOWN) {
				if (vsock_stream_has_data(vsk) <= 0) {
					sk->sk_state = SS_UNCONNECTED;
					sock_set_flag(sk, SOCK_DONE);
					sk->sk_state_change(sk);
				}
			}
		}
		err = copied;
	}
out_wait:
	finish_wait(sk_sleep(sk), &wait);
out:
	release_sock(sk);
	return err;
}