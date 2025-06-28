static void ax25_kill_by_device(struct net_device *dev)
{
	ax25_dev *ax25_dev;
	ax25_cb *s;
	if ((ax25_dev = ax25_dev_ax25dev(dev)) == NULL)
		return;
	spin_lock_bh(&ax25_list_lock);
again:
	ax25_for_each(s, &ax25_list) {
		if (s->ax25_dev == ax25_dev) {
			spin_unlock_bh(&ax25_list_lock);
			lock_sock(s->sk);
			s->ax25_dev = NULL;
			release_sock(s->sk);
			ax25_disconnect(s, ENETUNREACH);
			spin_lock_bh(&ax25_list_lock);
			goto again;
		}
	}
	spin_unlock_bh(&ax25_list_lock);
}