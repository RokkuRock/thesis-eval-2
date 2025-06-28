void ipv4_pktinfo_prepare(const struct sock *sk, struct sk_buff *skb)
{
	struct in_pktinfo *pktinfo = PKTINFO_SKB_CB(skb);
	bool prepare = (inet_sk(sk)->cmsg_flags & IP_CMSG_PKTINFO) ||
		       ipv6_sk_rxinfo(sk);
	if (prepare && skb_rtable(skb)) {
		if (pktinfo->ipi_ifindex == LOOPBACK_IFINDEX)
			pktinfo->ipi_ifindex = inet_iif(skb);
		pktinfo->ipi_spec_dst.s_addr = fib_compute_spec_dst(skb);
	} else {
		pktinfo->ipi_ifindex = 0;
		pktinfo->ipi_spec_dst.s_addr = 0;
	}
	skb_dst_drop(skb);
}