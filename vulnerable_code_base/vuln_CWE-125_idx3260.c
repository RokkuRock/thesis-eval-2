void skb_complete_tx_timestamp(struct sk_buff *skb,
			       struct skb_shared_hwtstamps *hwtstamps)
{
	struct sock *sk = skb->sk;
	if (!skb_may_tx_timestamp(sk, false))
		return;
	if (likely(atomic_inc_not_zero(&sk->sk_refcnt))) {
		*skb_hwtstamps(skb) = *hwtstamps;
		__skb_complete_tx_timestamp(skb, sk, SCM_TSTAMP_SND);
		sock_put(sk);
	}