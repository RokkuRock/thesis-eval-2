NOEXPORT void reload_config() {
    static int delay=10;  
#ifdef HAVE_CHROOT
    struct stat sb;
#endif  
    if(options_parse(CONF_RELOAD)) {
        s_log(LOG_ERR, "Failed to reload the configuration file");
        return;
    }
    unbind_ports();
    log_flush(LOG_MODE_BUFFER);
#ifdef HAVE_CHROOT
    if(global_options.chroot_dir && stat("/dev/log", &sb))
        log_close(SINK_OUTFILE);
    else
#endif  
        log_close(SINK_SYSLOG|SINK_OUTFILE);
    options_free();
    options_apply();
    log_open(SINK_SYSLOG|SINK_OUTFILE);
    log_flush(LOG_MODE_CONFIGURED);
    ui_config_reloaded();
    if(bind_ports() | exec_connect_start()) {
        s_poll_sleep(delay/1000, delay%1000);  
        signal_post(SIGNAL_RELOAD_CONFIG);  
        delay*=2;
        if(delay > 10000)  
            delay=10000;
    } else {
        delay=10;  
    }
}