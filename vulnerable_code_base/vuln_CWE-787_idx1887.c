int dns_add_HTTPS_start(struct dns_rr_nested *svcparam_buffer, struct dns_packet *packet, dns_rr_type type,
						const char *domain, int ttl, int priority, const char *target)
{
	svcparam_buffer = dns_add_rr_nested_start(svcparam_buffer, packet, type, DNS_T_HTTPS, domain, ttl);
	if (svcparam_buffer == NULL) {
		return -1;
	}
	int target_len = strnlen(target, DNS_MAX_CNAME_LEN) + 1;
	if (_dns_left_len(&svcparam_buffer->context) < 2 + target_len) {
		return -1;
	}
	_dns_write_short(&svcparam_buffer->context.ptr, priority);
	safe_strncpy((char *)svcparam_buffer->context.ptr, target, target_len);
	svcparam_buffer->context.ptr += target_len;
	return 0;
}