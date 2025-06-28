static int xen_netbk_tx_check_gop(struct xen_netbk *netbk,
				  struct sk_buff *skb,
				  struct gnttab_copy **gopp)
{
	struct gnttab_copy *gop = *gopp;
	u16 pending_idx = *((u16 *)skb->data);
	struct pending_tx_info *pending_tx_info = netbk->pending_tx_info;
	struct xenvif *vif = pending_tx_info[pending_idx].vif;
	struct xen_netif_tx_request *txp;
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	int nr_frags = shinfo->nr_frags;
	int i, err, start;
	err = gop->status;
	if (unlikely(err)) {
		pending_ring_idx_t index;
		index = pending_index(netbk->pending_prod++);
		txp = &pending_tx_info[pending_idx].req;
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		netbk->pending_ring[index] = pending_idx;
		xenvif_put(vif);
	}
	start = (frag_get_pending_idx(&shinfo->frags[0]) == pending_idx);
	for (i = start; i < nr_frags; i++) {
		int j, newerr;
		pending_ring_idx_t index;
		pending_idx = frag_get_pending_idx(&shinfo->frags[i]);
		newerr = (++gop)->status;
		if (likely(!newerr)) {
			if (unlikely(err))
				xen_netbk_idx_release(netbk, pending_idx);
			continue;
		}
		txp = &netbk->pending_tx_info[pending_idx].req;
		make_tx_response(vif, txp, XEN_NETIF_RSP_ERROR);
		index = pending_index(netbk->pending_prod++);
		netbk->pending_ring[index] = pending_idx;
		xenvif_put(vif);
		if (err)
			continue;
		pending_idx = *((u16 *)skb->data);
		xen_netbk_idx_release(netbk, pending_idx);
		for (j = start; j < i; j++) {
			pending_idx = frag_get_pending_idx(&shinfo->frags[j]);
			xen_netbk_idx_release(netbk, pending_idx);
		}
		err = newerr;
	}
	*gopp = gop + 1;
	return err;
}