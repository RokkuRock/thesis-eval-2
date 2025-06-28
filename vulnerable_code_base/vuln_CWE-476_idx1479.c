netdev_tx_t ieee80211_monitor_start_xmit(struct sk_buff *skb,
					 struct net_device *dev)
{
	struct ieee80211_local *local = wdev_priv(dev->ieee80211_ptr);
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr;
	struct ieee80211_sub_if_data *tmp_sdata, *sdata;
	struct cfg80211_chan_def *chandef;
	u16 len_rthdr;
	int hdrlen;
	memset(info, 0, sizeof(*info));
	info->flags = IEEE80211_TX_CTL_REQ_TX_STATUS |
		      IEEE80211_TX_CTL_INJECTED;
	if (!ieee80211_parse_tx_radiotap(skb, dev))
		goto fail;
	len_rthdr = ieee80211_get_radiotap_len(skb->data);
	skb_set_mac_header(skb, len_rthdr);
	skb_set_network_header(skb, len_rthdr);
	skb_set_transport_header(skb, len_rthdr);
	if (skb->len < len_rthdr + 2)
		goto fail;
	hdr = (struct ieee80211_hdr *)(skb->data + len_rthdr);
	hdrlen = ieee80211_hdrlen(hdr->frame_control);
	if (skb->len < len_rthdr + hdrlen)
		goto fail;
	if (ieee80211_is_data(hdr->frame_control) &&
	    skb->len >= len_rthdr + hdrlen + sizeof(rfc1042_header) + 2) {
		u8 *payload = (u8 *)hdr + hdrlen;
		if (ether_addr_equal(payload, rfc1042_header))
			skb->protocol = cpu_to_be16((payload[6] << 8) |
						    payload[7]);
	}
	rcu_read_lock();
	sdata = IEEE80211_DEV_TO_SUB_IF(dev);
	list_for_each_entry_rcu(tmp_sdata, &local->interfaces, list) {
		if (!ieee80211_sdata_running(tmp_sdata))
			continue;
		if (tmp_sdata->vif.type == NL80211_IFTYPE_MONITOR ||
		    tmp_sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			continue;
		if (ether_addr_equal(tmp_sdata->vif.addr, hdr->addr2)) {
			sdata = tmp_sdata;
			break;
		}
	}
	chanctx_conf = rcu_dereference(sdata->vif.chanctx_conf);
	if (!chanctx_conf) {
		tmp_sdata = rcu_dereference(local->monitor_sdata);
		if (tmp_sdata)
			chanctx_conf =
				rcu_dereference(tmp_sdata->vif.chanctx_conf);
	}
	if (chanctx_conf)
		chandef = &chanctx_conf->def;
	else if (!local->use_chanctx)
		chandef = &local->_oper_chandef;
	else
		goto fail_rcu;
	if (!cfg80211_reg_can_beacon(local->hw.wiphy, chandef,
				     sdata->vif.type))
		goto fail_rcu;
	info->band = chandef->chan->band;
	ieee80211_select_queue_80211(sdata, skb, hdr);
	skb_set_queue_mapping(skb, ieee80211_ac_from_tid(skb->priority));
	skb_pull(skb, len_rthdr);
	ieee80211_xmit(sdata, NULL, skb);
	rcu_read_unlock();
	return NETDEV_TX_OK;
fail_rcu:
	rcu_read_unlock();
fail:
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;  
}