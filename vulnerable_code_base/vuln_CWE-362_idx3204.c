int cipso_v4_req_setattr(struct request_sock *req,
			 const struct cipso_v4_doi *doi_def,
			 const struct netlbl_lsm_secattr *secattr)
{
	int ret_val = -EPERM;
	unsigned char *buf = NULL;
	u32 buf_len;
	u32 opt_len;
	struct ip_options *opt = NULL;
	struct inet_request_sock *req_inet;
	buf_len = CIPSO_V4_OPT_LEN_MAX;
	buf = kmalloc(buf_len, GFP_ATOMIC);
	if (buf == NULL) {
		ret_val = -ENOMEM;
		goto req_setattr_failure;
	}
	ret_val = cipso_v4_genopt(buf, buf_len, doi_def, secattr);
	if (ret_val < 0)
		goto req_setattr_failure;
	buf_len = ret_val;
	opt_len = (buf_len + 3) & ~3;
	opt = kzalloc(sizeof(*opt) + opt_len, GFP_ATOMIC);
	if (opt == NULL) {
		ret_val = -ENOMEM;
		goto req_setattr_failure;
	}
	memcpy(opt->__data, buf, buf_len);
	opt->optlen = opt_len;
	opt->cipso = sizeof(struct iphdr);
	kfree(buf);
	buf = NULL;
	req_inet = inet_rsk(req);
	opt = xchg(&req_inet->opt, opt);
	kfree(opt);
	return 0;
req_setattr_failure:
	kfree(buf);
	kfree(opt);
	return ret_val;
}