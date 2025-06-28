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
	struct sctp_bind_hashbucket *head;
	struct list_head tmplist;
	newsk->sk_sndbuf = oldsk->sk_sndbuf;
	newsk->sk_rcvbuf = oldsk->sk_rcvbuf;
	if (oldsp->do_auto_asconf) {
		memcpy(&tmplist, &newsp->auto_asconf_list, sizeof(tmplist));
		inet_sk_copy_descendant(newsk, oldsk);
		memcpy(&newsp->auto_asconf_list, &tmplist, sizeof(tmplist));
	} else
		inet_sk_copy_descendant(newsk, oldsk);
	newsp->ep = newep;
	newsp->hmac = NULL;
	head = &sctp_port_hashtable[sctp_phashfn(sock_net(oldsk),
						 inet_sk(oldsk)->inet_num)];
	local_bh_disable();
	spin_lock(&head->lock);
	pp = sctp_sk(oldsk)->bind_hash;
	sk_add_bind_node(newsk, &pp->owner);
	sctp_sk(newsk)->bind_hash = pp;
	inet_sk(newsk)->inet_num = inet_sk(oldsk)->inet_num;
	spin_unlock(&head->lock);
	local_bh_enable();
	sctp_bind_addr_dup(&newsp->ep->base.bind_addr,
				&oldsp->ep->base.bind_addr, GFP_KERNEL);
	sctp_skb_for_each(skb, &oldsk->sk_receive_queue, tmp) {
		event = sctp_skb2event(skb);
		if (event->asoc == assoc) {
			__skb_unlink(skb, &oldsk->sk_receive_queue);
			__skb_queue_tail(&newsk->sk_receive_queue, skb);
			sctp_skb_set_owner_r_frag(skb, newsk);
		}
	}
	skb_queue_head_init(&newsp->pd_lobby);
	atomic_set(&sctp_sk(newsk)->pd_mode, assoc->ulpq.pd_mode);
	if (atomic_read(&sctp_sk(oldsk)->pd_mode)) {
		struct sk_buff_head *queue;
		if (assoc->ulpq.pd_mode) {
			queue = &newsp->pd_lobby;
		} else
			queue = &newsk->sk_receive_queue;
		sctp_skb_for_each(skb, &oldsp->pd_lobby, tmp) {
			event = sctp_skb2event(skb);
			if (event->asoc == assoc) {
				__skb_unlink(skb, &oldsp->pd_lobby);
				__skb_queue_tail(queue, skb);
				sctp_skb_set_owner_r_frag(skb, newsk);
			}
		}
		if (assoc->ulpq.pd_mode)
			sctp_clear_pd(oldsk, NULL);
	}
	sctp_skb_for_each(skb, &assoc->ulpq.reasm, tmp)
		sctp_skb_set_owner_r_frag(skb, newsk);
	sctp_skb_for_each(skb, &assoc->ulpq.lobby, tmp)
		sctp_skb_set_owner_r_frag(skb, newsk);
	newsp->type = type;
	lock_sock_nested(newsk, SINGLE_DEPTH_NESTING);
	sctp_assoc_migrate(assoc, newsk);
	if (sctp_state(assoc, CLOSED) && sctp_style(newsk, TCP))
		newsk->sk_shutdown |= RCV_SHUTDOWN;
	newsk->sk_state = SCTP_SS_ESTABLISHED;
	release_sock(newsk);
}