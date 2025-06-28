static void ax25_kill_by_device(struct net_device *dev)
{
	ax25_dev *ax25_dev;
	ax25_cb *s;
	struct sock *sk;
	if ((ax25_dev = ax25_dev_ax25dev(dev)) == NULL)
		return;
	spin_lock_bh(&ax25_list_lock);
again:
	ax25_for_each(s, &ax25_list) {
		if (s->ax25_dev == ax25_dev) {
			sk = s->sk;
			sock_hold(sk);
			spin_unlock_bh(&ax25_list_lock);
			lock_sock(sk);
			s->ax25_dev = NULL;
			ax25_dev_put(ax25_dev);
			release_sock(sk);
			ax25_disconnect(s, ENETUNREACH);
			spin_lock_bh(&ax25_list_lock);
			sock_put(sk);
			goto again;
		}
	}
	spin_unlock_bh(&ax25_list_lock);
}