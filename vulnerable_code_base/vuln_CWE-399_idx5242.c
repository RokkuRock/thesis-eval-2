void serveloop(GArray* servers) {
	struct sockaddr_storage addrin;
	socklen_t addrinlen=sizeof(addrin);
	int i;
	int max;
	fd_set mset;
	fd_set rset;
	max=0;
	FD_ZERO(&mset);
	for(i=0;i<servers->len;i++) {
		int sock;
		if((sock=(g_array_index(servers, SERVER, i)).socket) >= 0) {
			FD_SET(sock, &mset);
			max=sock>max?sock:max;
		}
	}
	for(i=0;i<modernsocks->len;i++) {
		int sock = g_array_index(modernsocks, int, i);
		FD_SET(sock, &mset);
		max=sock>max?sock:max;
	}
	for(;;) {
                if (is_sighup_caught) {
                        int n;
                        GError *gerror = NULL;
                        msg(LOG_INFO, "reconfiguration request received");
                        is_sighup_caught = 0;  
                        n = append_new_servers(servers, &gerror);
                        if (n == -1)
                                msg(LOG_ERR, "failed to append new servers: %s",
                                    gerror->message);
                        for (i = servers->len - n; i < servers->len; ++i) {
                                const SERVER server = g_array_index(servers,
                                                                    SERVER, i);
                                if (server.socket >= 0) {
                                        FD_SET(server.socket, &mset);
                                        max = server.socket > max ? server.socket : max;
                                }
                                msg(LOG_INFO, "reconfigured new server: %s",
                                    server.servename);
                        }
                }
		memcpy(&rset, &mset, sizeof(fd_set));
		if(select(max+1, &rset, NULL, NULL, NULL)>0) {
			int net;
			DEBUG("accept, ");
			for(i=0; i < modernsocks->len; i++) {
				int sock = g_array_index(modernsocks, int, i);
				if(!FD_ISSET(sock, &rset)) {
					continue;
				}
				CLIENT *client;
				if((net=accept(sock, (struct sockaddr *) &addrin, &addrinlen)) < 0) {
					err_nonfatal("accept: %m");
					continue;
				}
				client = negotiate(net, NULL, servers, NEG_INIT | NEG_MODERN);
				if(!client) {
					close(net);
					continue;
				}
				handle_connection(servers, net, client->server, client);
			}
			for(i=0; i < servers->len; i++) {
				SERVER *serve;
				serve=&(g_array_index(servers, SERVER, i));
				if(serve->socket < 0) {
					continue;
				}
				if(FD_ISSET(serve->socket, &rset)) {
					if ((net=accept(serve->socket, (struct sockaddr *) &addrin, &addrinlen)) < 0) {
						err_nonfatal("accept: %m");
						continue;
					}
					handle_connection(servers, net, serve, NULL);
				}
			}
		}
	}
}