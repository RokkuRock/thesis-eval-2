int dccp_rcv_state_process(struct sock *sk, struct sk_buff *skb,
			   struct dccp_hdr *dh, unsigned int len)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct dccp_skb_cb *dcb = DCCP_SKB_CB(skb);
	const int old_state = sk->sk_state;
	int queued = 0;
	if (sk->sk_state == DCCP_LISTEN) {
		if (dh->dccph_type == DCCP_PKT_REQUEST) {
			if (inet_csk(sk)->icsk_af_ops->conn_request(sk,
								    skb) < 0)
				return 1;
			goto discard;
		}
		if (dh->dccph_type == DCCP_PKT_RESET)
			goto discard;
		dcb->dccpd_reset_code = DCCP_RESET_CODE_NO_CONNECTION;
		return 1;
	} else if (sk->sk_state == DCCP_CLOSED) {
		dcb->dccpd_reset_code = DCCP_RESET_CODE_NO_CONNECTION;
		return 1;
	}
	if (sk->sk_state != DCCP_REQUESTING && dccp_check_seqno(sk, skb))
		goto discard;
	if ((dp->dccps_role != DCCP_ROLE_CLIENT &&
	     dh->dccph_type == DCCP_PKT_RESPONSE) ||
	    (dp->dccps_role == DCCP_ROLE_CLIENT &&
	     dh->dccph_type == DCCP_PKT_REQUEST) ||
	    (sk->sk_state == DCCP_RESPOND && dh->dccph_type == DCCP_PKT_DATA)) {
		dccp_send_sync(sk, dcb->dccpd_seq, DCCP_PKT_SYNC);
		goto discard;
	}
	if (dccp_parse_options(sk, NULL, skb))
		return 1;
	if (dh->dccph_type == DCCP_PKT_RESET) {
		dccp_rcv_reset(sk, skb);
		return 0;
	} else if (dh->dccph_type == DCCP_PKT_CLOSEREQ) {	 
		if (dccp_rcv_closereq(sk, skb))
			return 0;
		goto discard;
	} else if (dh->dccph_type == DCCP_PKT_CLOSE) {		 
		if (dccp_rcv_close(sk, skb))
			return 0;
		goto discard;
	}
	switch (sk->sk_state) {
	case DCCP_REQUESTING:
		queued = dccp_rcv_request_sent_state_process(sk, skb, dh, len);
		if (queued >= 0)
			return queued;
		__kfree_skb(skb);
		return 0;
	case DCCP_PARTOPEN:
		dccp_handle_ackvec_processing(sk, skb);
		dccp_deliver_input_to_ccids(sk, skb);
	case DCCP_RESPOND:
		queued = dccp_rcv_respond_partopen_state_process(sk, skb,
								 dh, len);
		break;
	}
	if (dh->dccph_type == DCCP_PKT_ACK ||
	    dh->dccph_type == DCCP_PKT_DATAACK) {
		switch (old_state) {
		case DCCP_PARTOPEN:
			sk->sk_state_change(sk);
			sk_wake_async(sk, SOCK_WAKE_IO, POLL_OUT);
			break;
		}
	} else if (unlikely(dh->dccph_type == DCCP_PKT_SYNC)) {
		dccp_send_sync(sk, dcb->dccpd_seq, DCCP_PKT_SYNCACK);
		goto discard;
	}
	if (!queued) {
discard:
		__kfree_skb(skb);
	}
	return 0;
}