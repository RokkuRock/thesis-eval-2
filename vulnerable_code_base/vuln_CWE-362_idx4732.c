int cipso_v4_sock_setattr(struct sock *sk,
			  const struct cipso_v4_doi *doi_def,
			  const struct netlbl_lsm_secattr *secattr)
{
	int ret_val = -EPERM;
	unsigned char *buf = NULL;
	u32 buf_len;
	u32 opt_len;
	struct ip_options *opt = NULL;
	struct inet_sock *sk_inet;
	struct inet_connection_sock *sk_conn;
	if (sk == NULL)
		return 0;
	buf_len = CIPSO_V4_OPT_LEN_MAX;
	buf = kmalloc(buf_len, GFP_ATOMIC);
	if (buf == NULL) {
		ret_val = -ENOMEM;
		goto socket_setattr_failure;
	}
	ret_val = cipso_v4_genopt(buf, buf_len, doi_def, secattr);
	if (ret_val < 0)
		goto socket_setattr_failure;
	buf_len = ret_val;
	opt_len = (buf_len + 3) & ~3;
	opt = kzalloc(sizeof(*opt) + opt_len, GFP_ATOMIC);
	if (opt == NULL) {
		ret_val = -ENOMEM;
		goto socket_setattr_failure;
	}
	memcpy(opt->__data, buf, buf_len);
	opt->optlen = opt_len;
	opt->cipso = sizeof(struct iphdr);
	kfree(buf);
	buf = NULL;
	sk_inet = inet_sk(sk);
	if (sk_inet->is_icsk) {
		sk_conn = inet_csk(sk);
		if (sk_inet->opt)
			sk_conn->icsk_ext_hdr_len -= sk_inet->opt->optlen;
		sk_conn->icsk_ext_hdr_len += opt->optlen;
		sk_conn->icsk_sync_mss(sk, sk_conn->icsk_pmtu_cookie);
	}
	opt = xchg(&sk_inet->opt, opt);
	kfree(opt);
	return 0;
socket_setattr_failure:
	kfree(buf);
	kfree(opt);
	return ret_val;
}