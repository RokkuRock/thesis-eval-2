static void bond_setup(struct net_device *bond_dev)
{
	struct bonding *bond = netdev_priv(bond_dev);
	rwlock_init(&bond->lock);
	rwlock_init(&bond->curr_slave_lock);
	bond->params = bonding_defaults;
	bond->dev = bond_dev;
	INIT_LIST_HEAD(&bond->vlan_list);
	ether_setup(bond_dev);
	bond_dev->netdev_ops = &bond_netdev_ops;
	bond_dev->ethtool_ops = &bond_ethtool_ops;
	bond_set_mode_ops(bond, bond->params.mode);
	bond_dev->destructor = bond_destructor;
	bond_dev->tx_queue_len = 0;
	bond_dev->flags |= IFF_MASTER|IFF_MULTICAST;
	bond_dev->priv_flags |= IFF_BONDING;
	bond_dev->priv_flags &= ~IFF_XMIT_DST_RELEASE;
	bond_dev->features |= NETIF_F_VLAN_CHALLENGED;
	bond_dev->features |= NETIF_F_LLTX;
	bond_dev->hw_features = BOND_VLAN_FEATURES |
				NETIF_F_HW_VLAN_TX |
				NETIF_F_HW_VLAN_RX |
				NETIF_F_HW_VLAN_FILTER;
	bond_dev->hw_features &= ~(NETIF_F_ALL_CSUM & ~NETIF_F_NO_CSUM);
	bond_dev->features |= bond_dev->hw_features;
}