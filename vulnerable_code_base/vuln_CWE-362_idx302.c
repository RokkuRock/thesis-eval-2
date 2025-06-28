static struct sock * tcp_v6_syn_recv_sock(struct sock *sk, struct sk_buff *skb,
					  struct request_sock *req,
					  struct dst_entry *dst)
{
	struct inet6_request_sock *treq;
	struct ipv6_pinfo *newnp, *np = inet6_sk(sk);
	struct tcp6_sock *newtcp6sk;
	struct inet_sock *newinet;
	struct tcp_sock *newtp;
	struct sock *newsk;
	struct ipv6_txoptions *opt;
#ifdef CONFIG_TCP_MD5SIG
	struct tcp_md5sig_key *key;
#endif
	if (skb->protocol == htons(ETH_P_IP)) {
		newsk = tcp_v4_syn_recv_sock(sk, skb, req, dst);
		if (newsk == NULL)
			return NULL;
		newtcp6sk = (struct tcp6_sock *)newsk;
		inet_sk(newsk)->pinet6 = &newtcp6sk->inet6;
		newinet = inet_sk(newsk);
		newnp = inet6_sk(newsk);
		newtp = tcp_sk(newsk);
		memcpy(newnp, np, sizeof(struct ipv6_pinfo));
		ipv6_addr_set_v4mapped(newinet->inet_daddr, &newnp->daddr);
		ipv6_addr_set_v4mapped(newinet->inet_saddr, &newnp->saddr);
		ipv6_addr_copy(&newnp->rcv_saddr, &newnp->saddr);
		inet_csk(newsk)->icsk_af_ops = &ipv6_mapped;
		newsk->sk_backlog_rcv = tcp_v4_do_rcv;
#ifdef CONFIG_TCP_MD5SIG
		newtp->af_specific = &tcp_sock_ipv6_mapped_specific;
#endif
		newnp->pktoptions  = NULL;
		newnp->opt	   = NULL;
		newnp->mcast_oif   = inet6_iif(skb);
		newnp->mcast_hops  = ipv6_hdr(skb)->hop_limit;
		tcp_sync_mss(newsk, inet_csk(newsk)->icsk_pmtu_cookie);
		return newsk;
	}
	treq = inet6_rsk(req);
	opt = np->opt;
	if (sk_acceptq_is_full(sk))
		goto out_overflow;
	if (!dst) {
		dst = inet6_csk_route_req(sk, req);
		if (!dst)
			goto out;
	}
	newsk = tcp_create_openreq_child(sk, req, skb);
	if (newsk == NULL)
		goto out_nonewsk;
	newsk->sk_gso_type = SKB_GSO_TCPV6;
	__ip6_dst_store(newsk, dst, NULL, NULL);
	newtcp6sk = (struct tcp6_sock *)newsk;
	inet_sk(newsk)->pinet6 = &newtcp6sk->inet6;
	newtp = tcp_sk(newsk);
	newinet = inet_sk(newsk);
	newnp = inet6_sk(newsk);
	memcpy(newnp, np, sizeof(struct ipv6_pinfo));
	ipv6_addr_copy(&newnp->daddr, &treq->rmt_addr);
	ipv6_addr_copy(&newnp->saddr, &treq->loc_addr);
	ipv6_addr_copy(&newnp->rcv_saddr, &treq->loc_addr);
	newsk->sk_bound_dev_if = treq->iif;
	newinet->opt = NULL;
	newnp->ipv6_fl_list = NULL;
	newnp->rxopt.all = np->rxopt.all;
	newnp->pktoptions = NULL;
	if (treq->pktopts != NULL) {
		newnp->pktoptions = skb_clone(treq->pktopts, GFP_ATOMIC);
		kfree_skb(treq->pktopts);
		treq->pktopts = NULL;
		if (newnp->pktoptions)
			skb_set_owner_r(newnp->pktoptions, newsk);
	}
	newnp->opt	  = NULL;
	newnp->mcast_oif  = inet6_iif(skb);
	newnp->mcast_hops = ipv6_hdr(skb)->hop_limit;
	if (opt) {
		newnp->opt = ipv6_dup_options(newsk, opt);
		if (opt != np->opt)
			sock_kfree_s(sk, opt, opt->tot_len);
	}
	inet_csk(newsk)->icsk_ext_hdr_len = 0;
	if (newnp->opt)
		inet_csk(newsk)->icsk_ext_hdr_len = (newnp->opt->opt_nflen +
						     newnp->opt->opt_flen);
	tcp_mtup_init(newsk);
	tcp_sync_mss(newsk, dst_mtu(dst));
	newtp->advmss = dst_metric_advmss(dst);
	tcp_initialize_rcv_mss(newsk);
	newinet->inet_daddr = newinet->inet_saddr = LOOPBACK4_IPV6;
	newinet->inet_rcv_saddr = LOOPBACK4_IPV6;
#ifdef CONFIG_TCP_MD5SIG
	if ((key = tcp_v6_md5_do_lookup(sk, &newnp->daddr)) != NULL) {
		char *newkey = kmemdup(key->key, key->keylen, GFP_ATOMIC);
		if (newkey != NULL)
			tcp_v6_md5_do_add(newsk, &newnp->daddr,
					  newkey, key->keylen);
	}
#endif
	if (__inet_inherit_port(sk, newsk) < 0) {
		sock_put(newsk);
		goto out;
	}
	__inet6_hash(newsk, NULL);
	return newsk;
out_overflow:
	NET_INC_STATS_BH(sock_net(sk), LINUX_MIB_LISTENOVERFLOWS);
out_nonewsk:
	if (opt && opt != np->opt)
		sock_kfree_s(sk, opt, opt->tot_len);
	dst_release(dst);
out:
	NET_INC_STATS_BH(sock_net(sk), LINUX_MIB_LISTENDROPS);
	return NULL;
}