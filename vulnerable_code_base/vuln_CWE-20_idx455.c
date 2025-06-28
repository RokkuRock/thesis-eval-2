static int netbk_set_skb_gso(struct xenvif *vif,
			     struct sk_buff *skb,
			     struct xen_netif_extra_info *gso)
{
	if (!gso->u.gso.size) {
		netdev_dbg(vif->dev, "GSO size must not be zero.\n");
		return -EINVAL;
	}
	if (gso->u.gso.type != XEN_NETIF_GSO_TYPE_TCPV4) {
		netdev_dbg(vif->dev, "Bad GSO type %d.\n", gso->u.gso.type);
		return -EINVAL;
	}
	skb_shinfo(skb)->gso_size = gso->u.gso.size;
	skb_shinfo(skb)->gso_type = SKB_GSO_TCPV4;
	skb_shinfo(skb)->gso_type |= SKB_GSO_DODGY;
	skb_shinfo(skb)->gso_segs = 0;
	return 0;
}