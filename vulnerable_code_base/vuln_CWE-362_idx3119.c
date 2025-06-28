void sctp_generate_t3_rtx_event(unsigned long peer)
{
	int error;
	struct sctp_transport *transport = (struct sctp_transport *) peer;
	struct sctp_association *asoc = transport->asoc;
	struct net *net = sock_net(asoc->base.sk);
	bh_lock_sock(asoc->base.sk);
	if (sock_owned_by_user(asoc->base.sk)) {
		pr_debug("%s: sock is busy\n", __func__);
		if (!mod_timer(&transport->T3_rtx_timer, jiffies + (HZ/20)))
			sctp_transport_hold(transport);
		goto out_unlock;
	}
	if (transport->dead)
		goto out_unlock;
	error = sctp_do_sm(net, SCTP_EVENT_T_TIMEOUT,
			   SCTP_ST_TIMEOUT(SCTP_EVENT_TIMEOUT_T3_RTX),
			   asoc->state,
			   asoc->ep, asoc,
			   transport, GFP_ATOMIC);
	if (error)
		asoc->base.sk->sk_err = -error;
out_unlock:
	bh_unlock_sock(asoc->base.sk);
	sctp_transport_put(transport);
}