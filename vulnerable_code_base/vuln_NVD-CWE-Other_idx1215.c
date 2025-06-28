int tipc_link_xmit(struct tipc_link *l, struct sk_buff_head *list,
		   struct sk_buff_head *xmitq)
{
	struct tipc_msg *hdr = buf_msg(skb_peek(list));
	struct sk_buff_head *backlogq = &l->backlogq;
	struct sk_buff_head *transmq = &l->transmq;
	struct sk_buff *skb, *_skb;
	u16 bc_ack = l->bc_rcvlink->rcv_nxt - 1;
	u16 ack = l->rcv_nxt - 1;
	u16 seqno = l->snd_nxt;
	int pkt_cnt = skb_queue_len(list);
	int imp = msg_importance(hdr);
	unsigned int mss = tipc_link_mss(l);
	unsigned int cwin = l->window;
	unsigned int mtu = l->mtu;
	bool new_bundle;
	int rc = 0;
	if (unlikely(msg_size(hdr) > mtu)) {
		pr_warn("Too large msg, purging xmit list %d %d %d %d %d!\n",
			skb_queue_len(list), msg_user(hdr),
			msg_type(hdr), msg_size(hdr), mtu);
		__skb_queue_purge(list);
		return -EMSGSIZE;
	}
	if (unlikely(l->backlog[imp].len >= l->backlog[imp].limit)) {
		if (imp == TIPC_SYSTEM_IMPORTANCE) {
			pr_warn("%s<%s>, link overflow", link_rst_msg, l->name);
			return -ENOBUFS;
		}
		rc = link_schedule_user(l, hdr);
	}
	if (pkt_cnt > 1) {
		l->stats.sent_fragmented++;
		l->stats.sent_fragments += pkt_cnt;
	}
	while ((skb = __skb_dequeue(list))) {
		if (likely(skb_queue_len(transmq) < cwin)) {
			hdr = buf_msg(skb);
			msg_set_seqno(hdr, seqno);
			msg_set_ack(hdr, ack);
			msg_set_bcast_ack(hdr, bc_ack);
			_skb = skb_clone(skb, GFP_ATOMIC);
			if (!_skb) {
				kfree_skb(skb);
				__skb_queue_purge(list);
				return -ENOBUFS;
			}
			__skb_queue_tail(transmq, skb);
			tipc_link_set_skb_retransmit_time(skb, l);
			__skb_queue_tail(xmitq, _skb);
			TIPC_SKB_CB(skb)->ackers = l->ackers;
			l->rcv_unacked = 0;
			l->stats.sent_pkts++;
			seqno++;
			continue;
		}
		if (tipc_msg_try_bundle(l->backlog[imp].target_bskb, &skb,
					mss, l->addr, &new_bundle)) {
			if (skb) {
				l->backlog[imp].target_bskb = skb;
				l->backlog[imp].len++;
				__skb_queue_tail(backlogq, skb);
			} else {
				if (new_bundle) {
					l->stats.sent_bundles++;
					l->stats.sent_bundled++;
				}
				l->stats.sent_bundled++;
			}
			continue;
		}
		l->backlog[imp].target_bskb = NULL;
		l->backlog[imp].len += (1 + skb_queue_len(list));
		__skb_queue_tail(backlogq, skb);
		skb_queue_splice_tail_init(list, backlogq);
	}
	l->snd_nxt = seqno;
	return rc;
}