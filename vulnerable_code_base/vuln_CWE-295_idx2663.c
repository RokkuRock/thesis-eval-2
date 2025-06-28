int main_configure(char *arg1, char *arg2) {
    int cmdline_status;
    cmdline_status=options_cmdline(arg1, arg2);
    if(cmdline_status)  
        return cmdline_status;
    options_apply();
    str_canary_init();  
    log_open(SINK_SYSLOG);
    if(bind_ports())
        return 1;
#ifdef HAVE_CHROOT
    if(change_root())
        return 1;
#endif  
    if(drop_privileges(1))
        return 1;
    if(log_open(SINK_OUTFILE))
        return 1;
#ifndef USE_FORK
    num_clients=0;  
#endif
    log_flush(LOG_MODE_CONFIGURED);
    return 0;
}