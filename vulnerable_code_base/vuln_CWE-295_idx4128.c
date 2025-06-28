void main_init() {  
#ifdef USE_SYSTEMD
    int i;
    systemd_fds=sd_listen_fds(1);
    if(systemd_fds<0)
        fatal("systemd initialization failed");
    listen_fds_start=SD_LISTEN_FDS_START;
    for(i=0; i<systemd_fds; ++i)
        set_nonblock(listen_fds_start+i, 1);
#else
    systemd_fds=0;  
    listen_fds_start=3;  
#endif
    if(ssl_init())  
        fatal("TLS initialization failed");
    if(sthreads_init())  
        fatal("Threads initialization failed");
    options_defaults();
    options_apply();
#ifndef USE_FORK
    get_limits();  
#endif
    fds=s_poll_alloc();
    if(pipe_init(signal_pipe, "signal_pipe"))
        fatal("Signal pipe initialization failed: "
            "check your personal firewall");
    if(pipe_init(terminate_pipe, "terminate_pipe"))
        fatal("Terminate pipe initialization failed: "
            "check your personal firewall");
    stunnel_info(LOG_NOTICE);
    if(systemd_fds>0)
        s_log(LOG_INFO, "Systemd socket activation: %d descriptors received",
            systemd_fds);
}