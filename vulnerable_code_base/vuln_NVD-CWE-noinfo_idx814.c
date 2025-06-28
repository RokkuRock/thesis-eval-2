pf_setup_pdesc(struct pf_pdesc *pd, sa_family_t af, int dir,
    struct pfi_kif *kif, struct mbuf *m, u_short *reason)
{
	memset(pd, 0, sizeof(*pd));
	pd->dir = dir;
	pd->kif = kif;		 
	pd->m = m;
	pd->sidx = (dir == PF_IN) ? 0 : 1;
	pd->didx = (dir == PF_IN) ? 1 : 0;
	pd->af = pd->naf = af;
	pd->rdomain = rtable_l2(pd->m->m_pkthdr.ph_rtableid);
	switch (pd->af) {
	case AF_INET: {
		struct ip	*h;
		if (pd->m->m_pkthdr.len < (int)sizeof(struct ip)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		h = mtod(pd->m, struct ip *);
		if (pd->m->m_pkthdr.len < ntohs(h->ip_len)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		if (pf_walk_header(pd, h, reason) != PF_PASS)
			return (PF_DROP);
		pd->src = (struct pf_addr *)&h->ip_src;
		pd->dst = (struct pf_addr *)&h->ip_dst;
		pd->tot_len = ntohs(h->ip_len);
		pd->tos = h->ip_tos & ~IPTOS_ECN_MASK;
		pd->ttl = h->ip_ttl;
		pd->virtual_proto = (h->ip_off & htons(IP_MF | IP_OFFMASK)) ?
		     PF_VPROTO_FRAGMENT : pd->proto;
		break;
	}
#ifdef INET6
	case AF_INET6: {
		struct ip6_hdr	*h;
		if (pd->m->m_pkthdr.len < (int)sizeof(struct ip6_hdr)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		h = mtod(pd->m, struct ip6_hdr *);
		if (pd->m->m_pkthdr.len <
		    sizeof(struct ip6_hdr) + ntohs(h->ip6_plen)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		if (pf_walk_header6(pd, h, reason) != PF_PASS)
			return (PF_DROP);
#if 1
		if (pd->jumbolen != 0) {
			REASON_SET(reason, PFRES_NORM);
			return (PF_DROP);
		}
#endif	 
		pd->src = (struct pf_addr *)&h->ip6_src;
		pd->dst = (struct pf_addr *)&h->ip6_dst;
		pd->tot_len = ntohs(h->ip6_plen) + sizeof(struct ip6_hdr);
		pd->tos = (ntohl(h->ip6_flow) & 0x0fc00000) >> 20;
		pd->ttl = h->ip6_hlim;
		pd->virtual_proto = (pd->fragoff != 0) ?
			PF_VPROTO_FRAGMENT : pd->proto;
		break;
	}
#endif  
	default:
		panic("pf_setup_pdesc called with illegal af %u", pd->af);
	}
	pf_addrcpy(&pd->nsaddr, pd->src, pd->af);
	pf_addrcpy(&pd->ndaddr, pd->dst, pd->af);
	switch (pd->virtual_proto) {
	case IPPROTO_TCP: {
		struct tcphdr	*th = &pd->hdr.tcp;
		if (!pf_pull_hdr(pd->m, pd->off, th, sizeof(*th),
		    NULL, reason, pd->af))
			return (PF_DROP);
		pd->hdrlen = sizeof(*th);
		if (pd->off + (th->th_off << 2) > pd->tot_len ||
		    (th->th_off << 2) < sizeof(struct tcphdr)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		pd->p_len = pd->tot_len - pd->off - (th->th_off << 2);
		pd->sport = &th->th_sport;
		pd->dport = &th->th_dport;
		pd->pcksum = &th->th_sum;
		break;
	}
	case IPPROTO_UDP: {
		struct udphdr	*uh = &pd->hdr.udp;
		if (!pf_pull_hdr(pd->m, pd->off, uh, sizeof(*uh),
		    NULL, reason, pd->af))
			return (PF_DROP);
		pd->hdrlen = sizeof(*uh);
		if (uh->uh_dport == 0 ||
		    pd->off + ntohs(uh->uh_ulen) > pd->tot_len ||
		    ntohs(uh->uh_ulen) < sizeof(struct udphdr)) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		pd->sport = &uh->uh_sport;
		pd->dport = &uh->uh_dport;
		pd->pcksum = &uh->uh_sum;
		break;
	}
	case IPPROTO_ICMP: {
		if (!pf_pull_hdr(pd->m, pd->off, &pd->hdr.icmp, ICMP_MINLEN,
		    NULL, reason, pd->af))
			return (PF_DROP);
		pd->hdrlen = ICMP_MINLEN;
		if (pd->off + pd->hdrlen > pd->tot_len) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		pd->pcksum = &pd->hdr.icmp.icmp_cksum;
		break;
	}
#ifdef INET6
	case IPPROTO_ICMPV6: {
		size_t	icmp_hlen = sizeof(struct icmp6_hdr);
		if (!pf_pull_hdr(pd->m, pd->off, &pd->hdr.icmp6, icmp_hlen,
		    NULL, reason, pd->af))
			return (PF_DROP);
		switch (pd->hdr.icmp6.icmp6_type) {
		case MLD_LISTENER_QUERY:
		case MLD_LISTENER_REPORT:
			icmp_hlen = sizeof(struct mld_hdr);
			break;
		case ND_NEIGHBOR_SOLICIT:
		case ND_NEIGHBOR_ADVERT:
			icmp_hlen = sizeof(struct nd_neighbor_solicit);
		case ND_ROUTER_SOLICIT:
		case ND_ROUTER_ADVERT:
		case ND_REDIRECT:
			if (pd->ttl != 255) {
				REASON_SET(reason, PFRES_NORM);
				return (PF_DROP);
			}
			break;
		}
		if (icmp_hlen > sizeof(struct icmp6_hdr) &&
		    !pf_pull_hdr(pd->m, pd->off, &pd->hdr.icmp6, icmp_hlen,
		    NULL, reason, pd->af))
			return (PF_DROP);
		pd->hdrlen = icmp_hlen;
		if (pd->off + pd->hdrlen > pd->tot_len) {
			REASON_SET(reason, PFRES_SHORT);
			return (PF_DROP);
		}
		pd->pcksum = &pd->hdr.icmp6.icmp6_cksum;
		break;
	}
#endif	 
	}
	if (pd->sport)
		pd->osport = pd->nsport = *pd->sport;
	if (pd->dport)
		pd->odport = pd->ndport = *pd->dport;
	pd->hash = pf_pkt_hash(pd->af, pd->proto,
	    pd->src, pd->dst, pd->osport, pd->odport);
	return (PF_PASS);
}