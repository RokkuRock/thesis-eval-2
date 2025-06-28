ascii_load_sockaddr(struct sockaddr_storage *ss, char *buf)
{
	struct sockaddr_in6 ssin6;
	struct sockaddr_in  ssin;
	memset(&ssin, 0, sizeof ssin);
	memset(&ssin6, 0, sizeof ssin6);
	if (!strcmp("local", buf)) {
		ss->ss_family = AF_LOCAL;
	}
	else if (buf[0] == '[' && buf[strlen(buf)-1] == ']') {
		buf[strlen(buf)-1] = '\0';
		if (inet_pton(AF_INET6, buf+1, &ssin6.sin6_addr) != 1)
			return 0;
		ssin6.sin6_family = AF_INET6;
		memcpy(ss, &ssin6, sizeof(ssin6));
		ss->ss_len = sizeof(struct sockaddr_in6);
	}
	else {
		if (inet_pton(AF_INET, buf, &ssin.sin_addr) != 1)
			return 0;
		ssin.sin_family = AF_INET;
		memcpy(ss, &ssin, sizeof(ssin));
		ss->ss_len = sizeof(struct sockaddr_in);
	}
	return 1;
}