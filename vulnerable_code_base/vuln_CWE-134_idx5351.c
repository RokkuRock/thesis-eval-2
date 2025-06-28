int http_connect(int sockfd, const char *host, int port, AyProxyData *proxy)
{
	char cmd[512];
	char *inputline = NULL;
	char *proxy_auth = NULL;
	char debug_buff[512];
	int remaining = sizeof(cmd) - 1;
	remaining -= snprintf(cmd, sizeof(cmd), "CONNECT %s:%d HTTP/1.1\r\n", host, port);
	if (proxy->username && proxy->username[0]) {
		proxy_auth = encode_proxy_auth_str(proxy);
		strncat(cmd, "Proxy-Authorization: Basic ", remaining);
		remaining -= 27;
		strncat(cmd, proxy_auth, remaining);
		remaining -= strlen(proxy_auth);
		strncat(cmd, "\r\n", remaining);
		remaining -= 2;
	}
	strncat(cmd, "\r\n", remaining);
#ifndef DEBUG
	snprintf(debug_buff, sizeof(debug_buff), "<%s>\n", cmd);
	debug_print(debug_buff);
#endif
	if (send(sockfd, cmd, strlen(cmd), 0) < 0)
		return AY_CONNECTION_REFUSED;
	if (ay_recv_line(sockfd, &inputline) < 0)
		return AY_CONNECTION_REFUSED;
#ifndef DEBUG
	snprintf(debug_buff, sizeof(debug_buff), "<%s>\n", inputline);
	debug_print(debug_buff);
#endif
	if (!strstr(inputline, "200")) {
		if (strstr(inputline, "407")) {
			while (ay_recv_line(sockfd, &inputline) > 0) {
				free(inputline);
			}
			return AY_PROXY_AUTH_REQUIRED;
		}
		if (strstr(inputline, "403")) {
			while (ay_recv_line(sockfd, &inputline) > 0) {
				free(inputline);
			}
			return AY_PROXY_PERMISSION_DENIED;
		}
		free(inputline);
		return AY_CONNECTION_REFUSED;
	}
	while (strlen(inputline) > 1) {
		free(inputline);
		if (ay_recv_line(sockfd, &inputline) < 0) {
			return AY_CONNECTION_REFUSED;
		}
#ifndef DEBUG
		snprintf(debug_buff, sizeof(debug_buff), "<%s>\n", inputline);
		debug_print(debug_buff);
#endif
	}
	free(inputline);
	g_free(proxy_auth);
	return 0;
}