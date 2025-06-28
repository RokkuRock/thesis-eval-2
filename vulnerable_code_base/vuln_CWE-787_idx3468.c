static int _dns_debug_display(struct dns_packet *packet)
{
	int i = 0;
	int j = 0;
	int ttl = 0;
	struct dns_rrs *rrs = NULL;
	int rr_count = 0;
	char req_host[MAX_IP_LEN];
	for (j = 1; j < DNS_RRS_END; j++) {
		rrs = dns_get_rrs_start(packet, j, &rr_count);
		printf("section: %d\n", j);
		for (i = 0; i < rr_count && rrs; i++, rrs = dns_get_rrs_next(packet, rrs)) {
			switch (rrs->type) {
			case DNS_T_A: {
				unsigned char addr[4];
				char name[DNS_MAX_CNAME_LEN] = {0};
				dns_get_A(rrs, name, DNS_MAX_CNAME_LEN, &ttl, addr);
				req_host[0] = '\0';
				inet_ntop(AF_INET, addr, req_host, sizeof(req_host));
				printf("domain: %s A: %s TTL: %d\n", name, req_host, ttl);
			} break;
			case DNS_T_AAAA: {
				unsigned char addr[16];
				char name[DNS_MAX_CNAME_LEN] = {0};
				dns_get_AAAA(rrs, name, DNS_MAX_CNAME_LEN, &ttl, addr);
				req_host[0] = '\0';
				inet_ntop(AF_INET6, addr, req_host, sizeof(req_host));
				printf("domain: %s AAAA: %s TTL:%d\n", name, req_host, ttl);
			} break;
			case DNS_T_HTTPS: {
				char name[DNS_MAX_CNAME_LEN] = {0};
				char target[DNS_MAX_CNAME_LEN] = {0};
				struct dns_https_param *p = NULL;
				int priority = 0;
				p = dns_get_HTTPS_svcparm_start(rrs, name, DNS_MAX_CNAME_LEN, &ttl, &priority, target,
												DNS_MAX_CNAME_LEN);
				if (p == NULL) {
					printf("get HTTPS svcparm failed\n");
					break;
				}
				printf("domain: %s HTTPS: %s TTL: %d priority: %d\n", name, target, ttl, priority);
				for (; p; p = dns_get_HTTPS_svcparm_next(rrs, p)) {
					switch (p->key) {
					case DNS_HTTPS_T_MANDATORY: {
						printf("  HTTPS: mandatory: %s\n", p->value);
					} break;
					case DNS_HTTPS_T_ALPN: {
						printf("  HTTPS: alpn: %s\n", p->value);
					} break;
					case DNS_HTTPS_T_NO_DEFAULT_ALPN: {
						printf("  HTTPS: no_default_alpn: %s\n", p->value);
					} break;
					case DNS_HTTPS_T_PORT: {
						int port = *(unsigned short *)(p->value);
						printf("  HTTPS: port: %d\n", port);
					} break;
					case DNS_HTTPS_T_IPV4HINT: {
						printf("  HTTPS: ipv4hint: %d\n", p->len / 4);
						for (int k = 0; k < p->len / 4; k++) {
							char ip[16] = {0};
							inet_ntop(AF_INET, p->value + k * 4, ip, sizeof(ip));
							printf("    ipv4: %s\n", ip);
						}
					} break;
					case DNS_HTTPS_T_ECH: {
						printf("  HTTPS: ech: ");
						for (int k = 0; k < p->len; k++) {
							printf("%02x ", p->value[k]);
						}
						printf("\n");
					} break;
					case DNS_HTTPS_T_IPV6HINT: {
						printf("  HTTPS: ipv6hint: %d\n", p->len / 16);
						for (int k = 0; k < p->len / 16; k++) {
							char ip[64] = {0};
							inet_ntop(AF_INET6, p->value + k * 16, ip, sizeof(ip));
							printf("    ipv6: %s\n", ip);
						}
					} break;
					}
				}
			} break;
			case DNS_T_NS: {
				char cname[DNS_MAX_CNAME_LEN];
				char name[DNS_MAX_CNAME_LEN] = {0};
				dns_get_CNAME(rrs, name, DNS_MAX_CNAME_LEN, &ttl, cname, DNS_MAX_CNAME_LEN);
				printf("domain: %s TTL: %d NS: %s\n", name, ttl, cname);
			} break;
			case DNS_T_CNAME: {
				char cname[DNS_MAX_CNAME_LEN];
				char name[DNS_MAX_CNAME_LEN] = {0};
				if (dns_conf_force_no_cname) {
					continue;
				}
				dns_get_CNAME(rrs, name, DNS_MAX_CNAME_LEN, &ttl, cname, DNS_MAX_CNAME_LEN);
				printf("domain: %s TTL: %d CNAME: %s\n", name, ttl, cname);
			} break;
			case DNS_T_SOA: {
				char name[DNS_MAX_CNAME_LEN] = {0};
				struct dns_soa soa;
				dns_get_SOA(rrs, name, 128, &ttl, &soa);
				printf("domain: %s SOA: mname: %s, rname: %s, serial: %d, refresh: %d, retry: %d, expire: "
					   "%d, minimum: %d",
					   name, soa.mname, soa.rname, soa.serial, soa.refresh, soa.retry, soa.expire, soa.minimum);
			} break;
			default:
				break;
			}
		}
		printf("\n");
	}
	return 0;
}