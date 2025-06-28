struct dns_https_param *dns_get_HTTPS_svcparm_start(struct dns_rrs *rrs, char *domain, int maxsize, int *ttl,
													int *priority, char *target, int target_size)
{
	int qtype = 0;
	unsigned char *data = NULL;
	int rr_len = 0;
	data = dns_get_rr_nested_start(rrs, domain, maxsize, &qtype, ttl, &rr_len);
	if (data == NULL) {
		return NULL;
	}
	if (qtype != DNS_T_HTTPS) {
		return NULL;
	}
	if (rr_len < 2) {
		return NULL;
	}
	*priority = _dns_read_short(&data);
	rr_len -= 2;
	if (rr_len <= 0) {
		return NULL;
	}
	int len = strnlen((char *)data, rr_len);
	safe_strncpy(target, (char *)data, target_size);
	data += len + 1;
	rr_len -= len + 1;
	if (rr_len <= 0) {
		return NULL;
	}
	return (struct dns_https_param *)data;
}