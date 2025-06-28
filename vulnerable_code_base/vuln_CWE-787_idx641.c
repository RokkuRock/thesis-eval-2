static int _dns_encode_HTTPS(struct dns_context *context, struct dns_rrs *rrs)
{
	int ret = 0;
	int qtype = 0;
	int qclass = 0;
	char domain[DNS_MAX_CNAME_LEN];
	char target[DNS_MAX_CNAME_LEN] = {0};
	unsigned char *rr_len_ptr = NULL;
	unsigned char *start = NULL;
	unsigned char *rr_start = NULL;
	int ttl = 0;
	int priority = 0;
	struct dns_https_param *param = NULL;
	param = dns_get_HTTPS_svcparm_start(rrs, domain, DNS_MAX_CNAME_LEN, &ttl, &priority, target, DNS_MAX_CNAME_LEN);
	if (param == NULL) {
		tlog(TLOG_ERROR, "get https param failed.");
		return -1;
	}
	ret = _dns_encode_rr_head(context, domain, qtype, qclass, ttl, 0, &rr_len_ptr);
	if (ret < 0) {
		return -1;
	}
	rr_start = context->ptr;
	if (_dns_left_len(context) < 2) {
		tlog(TLOG_ERROR, "left len is invalid.");
		return -1;
	}
	_dns_write_short(&context->ptr, priority);
	ret = _dns_encode_domain(context, target);
	if (ret < 0) {
		return -1;
	}
	start = context->ptr;
	for (; param != NULL; param = dns_get_HTTPS_svcparm_next(rrs, param)) {
		if (context->ptr - start > rrs->len || _dns_left_len(context) <= 0) {
			return -1;
		}
		_dns_write_short(&context->ptr, param->key);
		_dns_write_short(&context->ptr, param->len);
		switch (param->key) {
		case DNS_HTTPS_T_MANDATORY:
		case DNS_HTTPS_T_NO_DEFAULT_ALPN:
		case DNS_HTTPS_T_ALPN:
		case DNS_HTTPS_T_PORT:
		case DNS_HTTPS_T_IPV4HINT:
		case DNS_HTTPS_T_ECH:
		case DNS_HTTPS_T_IPV6HINT: {
			memcpy(context->ptr, param->value, param->len);
			context->ptr += param->len;
		} break;
		default:
			context->ptr -= 4;
			break;
		}
	}
	_dns_write_short(&rr_len_ptr, context->ptr - rr_start);
	return 0;
}