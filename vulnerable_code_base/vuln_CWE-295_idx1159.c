int bind_ports(void) {
    SERVICE_OPTIONS *opt;
    int listening_section;
#ifdef USE_LIBWRAP
    libwrap_init();
#endif  
    s_poll_init(fds, 1);
    for(opt=service_options.next; opt; opt=opt->next) {
        unsigned i;
        for(i=0; i<opt->local_addr.num; ++i)
            opt->local_fd[i]=INVALID_SOCKET;
    }
    listening_section=0;
    for(opt=service_options.next; opt; opt=opt->next) {
        opt->bound_ports=0;
        if(opt->local_addr.num) {  
            unsigned i;
            s_log(LOG_DEBUG, "Binding service [%s]", opt->servname);
            for(i=0; i<opt->local_addr.num; ++i) {
                SOCKET fd;
                fd=bind_port(opt, listening_section, i);
                opt->local_fd[i]=fd;
                if(fd!=INVALID_SOCKET) {
                    s_poll_add(fds, fd, 1, 0);
                    ++opt->bound_ports;
                }
            }
            if(!opt->bound_ports) {
                s_log(LOG_ERR, "Binding service [%s] failed", opt->servname);
                return 1;
            }
            ++listening_section;
        } else if(opt->exec_name && opt->connect_addr.names) {
            s_log(LOG_DEBUG, "Skipped exec+connect service [%s]", opt->servname);
#ifndef OPENSSL_NO_TLSEXT
        } else if(!opt->option.client && opt->sni) {
            s_log(LOG_DEBUG, "Skipped SNI slave service [%s]", opt->servname);
#endif
        } else {  
            s_log(LOG_ERR, "Invalid service [%s]", opt->servname);
            return 1;
        }
    }
    if(listening_section<systemd_fds) {
        s_log(LOG_ERR,
            "Too many listening file descriptors received from systemd, got %d",
            systemd_fds);
        return 1;
    }
    return 0;  
}