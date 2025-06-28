static void sctp_sock_migrate(struct sock *oldsk, struct sock *newsk,
			      struct sctp_association *assoc,
			      sctp_socket_type_t type)
{
	struct sctp_sock *oldsp = sctp_sk(oldsk);
	struct sctp_sock *newsp = sctp_sk(newsk);
	struct sctp_bind_bucket *pp;  
	struct sctp_endpoint *newep = newsp->ep;
	struct sk_buff *skb, *tmp;
	struct sctp_ulpevent *event;
	int flags = 0;
	newsk->sk_sndbuf = oldsk->sk_sndbuf;
	newsk->sk_rcvbuf = oldsk->sk_rcvbuf;
	inet_sk_copy_descendant(newsk, oldsk);
	newsp->ep = newep;
	newsp->hmac = NULL;
	pp = sctp_sk(oldsk)->bind_hash;
	sk_add_bind_node(newsk, &pp->owner);
	sctp_sk(newsk)->bind_hash = pp;
	inet_sk(newsk)->num = inet_sk(oldsk)->num;
	if (PF_INET6 == assoc->base.sk->sk_family)
		flags = SCTP_ADDR6_ALLOWED;
	if (assoc->peer.ipv4_address)
		flags |= SCTP_ADDR4_PEERSUPP;
	if (assoc->peer.ipv6_address)
		flags |= SCTP_ADDR6_PEERSUPP;
	sctp_bind_addr_copy(&newsp->ep->base.bind_addr,
			     &oldsp->ep->base.bind_addr,
			     SCTP_SCOPE_GLOBAL, GFP_KERNEL, flags);
	sctp_skb_for_each(skb, &oldsk->sk_receive_queue, tmp) {
		event = sctp_skb2event(skb);
		if (event->asoc == assoc) {
			sctp_sock_rfree(skb);
			__skb_unlink(skb, &oldsk->sk_receive_queue);
			__skb_queue_tail(&newsk->sk_receive_queue, skb);
			sctp_skb_set_owner_r(skb, newsk);
		}
	}
	skb_queue_head_init(&newsp->pd_lobby);
	sctp_sk(newsk)->pd_mode = assoc->ulpq.pd_mode;
	if (sctp_sk(oldsk)->pd_mode) {
		struct sk_buff_head *queue;
		if (assoc->ulpq.pd_mode) {
			queue = &newsp->pd_lobby;
		} else
			queue = &newsk->sk_receive_queue;
		sctp_skb_for_each(skb, &oldsp->pd_lobby, tmp) {
			event = sctp_skb2event(skb);
			if (event->asoc == assoc) {
				sctp_sock_rfree(skb);
				__skb_unlink(skb, &oldsp->pd_lobby);
				__skb_queue_tail(queue, skb);
				sctp_skb_set_owner_r(skb, newsk);
			}
		}
		if (assoc->ulpq.pd_mode)
			sctp_clear_pd(oldsk);
	}
	newsp->type = type;
	sctp_lock_sock(newsk);
	sctp_assoc_migrate(assoc, newsk);
	if (sctp_state(assoc, CLOSED) && sctp_style(newsk, TCP))
		newsk->sk_shutdown |= RCV_SHUTDOWN;
	newsk->sk_state = SCTP_SS_ESTABLISHED;
	sctp_release_sock(newsk);
}