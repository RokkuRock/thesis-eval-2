static int __net_init sctp_net_init(struct net *net)
{
	int status;
	net->sctp.rto_initial			= SCTP_RTO_INITIAL;
	net->sctp.rto_min	 		= SCTP_RTO_MIN;
	net->sctp.rto_max 			= SCTP_RTO_MAX;
	net->sctp.rto_alpha			= SCTP_RTO_ALPHA;
	net->sctp.rto_beta			= SCTP_RTO_BETA;
	net->sctp.valid_cookie_life		= SCTP_DEFAULT_COOKIE_LIFE;
	net->sctp.cookie_preserve_enable 	= 1;
#if defined (CONFIG_SCTP_DEFAULT_COOKIE_HMAC_MD5)
	net->sctp.sctp_hmac_alg			= "md5";
#elif defined (CONFIG_SCTP_DEFAULT_COOKIE_HMAC_SHA1)
	net->sctp.sctp_hmac_alg			= "sha1";
#else
	net->sctp.sctp_hmac_alg			= NULL;
#endif
	net->sctp.max_burst			= SCTP_DEFAULT_MAX_BURST;
	net->sctp.max_retrans_association	= 10;
	net->sctp.max_retrans_path		= 5;
	net->sctp.max_retrans_init		= 8;
	net->sctp.sndbuf_policy			= 0;
	net->sctp.rcvbuf_policy			= 0;
	net->sctp.hb_interval			= SCTP_DEFAULT_TIMEOUT_HEARTBEAT;
	net->sctp.sack_timeout			= SCTP_DEFAULT_TIMEOUT_SACK;
	net->sctp.addip_enable = 0;
	net->sctp.addip_noauth = 0;
	net->sctp.default_auto_asconf = 0;
	net->sctp.prsctp_enable = 1;
	net->sctp.auth_enable = 0;
	net->sctp.scope_policy = SCTP_SCOPE_POLICY_ENABLE;
	net->sctp.rwnd_upd_shift = SCTP_DEFAULT_RWND_SHIFT;
	net->sctp.max_autoclose		= INT_MAX / HZ;
	status = sctp_sysctl_net_register(net);
	if (status)
		goto err_sysctl_register;
	status = init_sctp_mibs(net);
	if (status)
		goto err_init_mibs;
	status = sctp_proc_init(net);
	if (status)
		goto err_init_proc;
	sctp_dbg_objcnt_init(net);
	if ((status = sctp_ctl_sock_init(net))) {
		pr_err("Failed to initialize the SCTP control sock\n");
		goto err_ctl_sock_init;
	}
	INIT_LIST_HEAD(&net->sctp.local_addr_list);
	spin_lock_init(&net->sctp.local_addr_lock);
	sctp_get_local_addr_list(net);
	INIT_LIST_HEAD(&net->sctp.addr_waitq);
	INIT_LIST_HEAD(&net->sctp.auto_asconf_splist);
	spin_lock_init(&net->sctp.addr_wq_lock);
	net->sctp.addr_wq_timer.expires = 0;
	setup_timer(&net->sctp.addr_wq_timer, sctp_addr_wq_timeout_handler,
		    (unsigned long)net);
	return 0;
err_ctl_sock_init:
	sctp_dbg_objcnt_exit(net);
	sctp_proc_exit(net);
err_init_proc:
	cleanup_sctp_mibs(net);
err_init_mibs:
	sctp_sysctl_net_unregister(net);
err_sysctl_register:
	return status;
}