int dns_HTTPS_add_ipv6hint(struct dns_rr_nested *svcparam, unsigned char addr[][DNS_RR_AAAA_LEN], int addr_num)
{
	if (_dns_left_len(&svcparam->context) < 4 + addr_num * DNS_RR_AAAA_LEN) {
		return -1;
	}
	unsigned short value = DNS_HTTPS_T_IPV6HINT;
	dns_add_rr_nested_memcpy(svcparam, &value, 2);
	value = addr_num * DNS_RR_AAAA_LEN;
	dns_add_rr_nested_memcpy(svcparam, &value, 2);
	for (int i = 0; i < addr_num; i++) {
		dns_add_rr_nested_memcpy(svcparam, addr[i], DNS_RR_AAAA_LEN);
	}
	return 0;
}