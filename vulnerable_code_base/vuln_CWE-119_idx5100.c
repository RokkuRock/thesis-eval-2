int sock_setsockopt(struct socket *sock, int level, int optname,
		    char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	int val;
	int valbool;
	struct linger ling;
	int ret = 0;
	if (optname == SO_BINDTODEVICE)
		return sock_bindtodevice(sk, optval, optlen);
	if (optlen < sizeof(int))
		return -EINVAL;
	if (get_user(val, (int __user *)optval))
		return -EFAULT;
	valbool = val ? 1 : 0;
	lock_sock(sk);
	switch (optname) {
	case SO_DEBUG:
		if (val && !capable(CAP_NET_ADMIN))
			ret = -EACCES;
		else
			sock_valbool_flag(sk, SOCK_DBG, valbool);
		break;
	case SO_REUSEADDR:
		sk->sk_reuse = (valbool ? SK_CAN_REUSE : SK_NO_REUSE);
		break;
	case SO_TYPE:
	case SO_PROTOCOL:
	case SO_DOMAIN:
	case SO_ERROR:
		ret = -ENOPROTOOPT;
		break;
	case SO_DONTROUTE:
		sock_valbool_flag(sk, SOCK_LOCALROUTE, valbool);
		break;
	case SO_BROADCAST:
		sock_valbool_flag(sk, SOCK_BROADCAST, valbool);
		break;
	case SO_SNDBUF:
		if (val > sysctl_wmem_max)
			val = sysctl_wmem_max;
set_sndbuf:
		sk->sk_userlocks |= SOCK_SNDBUF_LOCK;
		if ((val * 2) < SOCK_MIN_SNDBUF)
			sk->sk_sndbuf = SOCK_MIN_SNDBUF;
		else
			sk->sk_sndbuf = val * 2;
		sk->sk_write_space(sk);
		break;
	case SO_SNDBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		goto set_sndbuf;
	case SO_RCVBUF:
		if (val > sysctl_rmem_max)
			val = sysctl_rmem_max;
set_rcvbuf:
		sk->sk_userlocks |= SOCK_RCVBUF_LOCK;
		if ((val * 2) < SOCK_MIN_RCVBUF)
			sk->sk_rcvbuf = SOCK_MIN_RCVBUF;
		else
			sk->sk_rcvbuf = val * 2;
		break;
	case SO_RCVBUFFORCE:
		if (!capable(CAP_NET_ADMIN)) {
			ret = -EPERM;
			break;
		}
		goto set_rcvbuf;
	case SO_KEEPALIVE:
#ifdef CONFIG_INET
		if (sk->sk_protocol == IPPROTO_TCP)
			tcp_set_keepalive(sk, valbool);
#endif
		sock_valbool_flag(sk, SOCK_KEEPOPEN, valbool);
		break;
	case SO_OOBINLINE:
		sock_valbool_flag(sk, SOCK_URGINLINE, valbool);
		break;
	case SO_NO_CHECK:
		sk->sk_no_check = valbool;
		break;
	case SO_PRIORITY:
		if ((val >= 0 && val <= 6) || capable(CAP_NET_ADMIN))
			sk->sk_priority = val;
		else
			ret = -EPERM;
		break;
	case SO_LINGER:
		if (optlen < sizeof(ling)) {
			ret = -EINVAL;	 
			break;
		}
		if (copy_from_user(&ling, optval, sizeof(ling))) {
			ret = -EFAULT;
			break;
		}
		if (!ling.l_onoff)
			sock_reset_flag(sk, SOCK_LINGER);
		else {
#if (BITS_PER_LONG == 32)
			if ((unsigned int)ling.l_linger >= MAX_SCHEDULE_TIMEOUT/HZ)
				sk->sk_lingertime = MAX_SCHEDULE_TIMEOUT;
			else
#endif
				sk->sk_lingertime = (unsigned int)ling.l_linger * HZ;
			sock_set_flag(sk, SOCK_LINGER);
		}
		break;
	case SO_BSDCOMPAT:
		sock_warn_obsolete_bsdism("setsockopt");
		break;
	case SO_PASSCRED:
		if (valbool)
			set_bit(SOCK_PASSCRED, &sock->flags);
		else
			clear_bit(SOCK_PASSCRED, &sock->flags);
		break;
	case SO_TIMESTAMP:
	case SO_TIMESTAMPNS:
		if (valbool)  {
			if (optname == SO_TIMESTAMP)
				sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
			else
				sock_set_flag(sk, SOCK_RCVTSTAMPNS);
			sock_set_flag(sk, SOCK_RCVTSTAMP);
			sock_enable_timestamp(sk, SOCK_TIMESTAMP);
		} else {
			sock_reset_flag(sk, SOCK_RCVTSTAMP);
			sock_reset_flag(sk, SOCK_RCVTSTAMPNS);
		}
		break;
	case SO_TIMESTAMPING:
		if (val & ~SOF_TIMESTAMPING_MASK) {
			ret = -EINVAL;
			break;
		}
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_TX_HARDWARE,
				  val & SOF_TIMESTAMPING_TX_HARDWARE);
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_TX_SOFTWARE,
				  val & SOF_TIMESTAMPING_TX_SOFTWARE);
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_RX_HARDWARE,
				  val & SOF_TIMESTAMPING_RX_HARDWARE);
		if (val & SOF_TIMESTAMPING_RX_SOFTWARE)
			sock_enable_timestamp(sk,
					      SOCK_TIMESTAMPING_RX_SOFTWARE);
		else
			sock_disable_timestamp(sk,
					       (1UL << SOCK_TIMESTAMPING_RX_SOFTWARE));
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_SOFTWARE,
				  val & SOF_TIMESTAMPING_SOFTWARE);
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_SYS_HARDWARE,
				  val & SOF_TIMESTAMPING_SYS_HARDWARE);
		sock_valbool_flag(sk, SOCK_TIMESTAMPING_RAW_HARDWARE,
				  val & SOF_TIMESTAMPING_RAW_HARDWARE);
		break;
	case SO_RCVLOWAT:
		if (val < 0)
			val = INT_MAX;
		sk->sk_rcvlowat = val ? : 1;
		break;
	case SO_RCVTIMEO:
		ret = sock_set_timeout(&sk->sk_rcvtimeo, optval, optlen);
		break;
	case SO_SNDTIMEO:
		ret = sock_set_timeout(&sk->sk_sndtimeo, optval, optlen);
		break;
	case SO_ATTACH_FILTER:
		ret = -EINVAL;
		if (optlen == sizeof(struct sock_fprog)) {
			struct sock_fprog fprog;
			ret = -EFAULT;
			if (copy_from_user(&fprog, optval, sizeof(fprog)))
				break;
			ret = sk_attach_filter(&fprog, sk);
		}
		break;
	case SO_DETACH_FILTER:
		ret = sk_detach_filter(sk);
		break;
	case SO_PASSSEC:
		if (valbool)
			set_bit(SOCK_PASSSEC, &sock->flags);
		else
			clear_bit(SOCK_PASSSEC, &sock->flags);
		break;
	case SO_MARK:
		if (!capable(CAP_NET_ADMIN))
			ret = -EPERM;
		else
			sk->sk_mark = val;
		break;
	case SO_RXQ_OVFL:
		sock_valbool_flag(sk, SOCK_RXQ_OVFL, valbool);
		break;
	case SO_WIFI_STATUS:
		sock_valbool_flag(sk, SOCK_WIFI_STATUS, valbool);
		break;
	case SO_PEEK_OFF:
		if (sock->ops->set_peek_off)
			sock->ops->set_peek_off(sk, val);
		else
			ret = -EOPNOTSUPP;
		break;
	case SO_NOFCS:
		sock_valbool_flag(sk, SOCK_NOFCS, valbool);
		break;
	default:
		ret = -ENOPROTOOPT;
		break;
	}
	release_sock(sk);
	return ret;
}