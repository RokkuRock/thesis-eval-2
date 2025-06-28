NOEXPORT unsigned __stdcall daemon_thread(void *arg) {
    (void)arg;  
    tls_alloc(NULL, NULL, "main");  
    main_init();
    SetEvent(main_initialized);  
    while(main_configure(cmdline.config_file, NULL)) {
        if(cmdline.config_file && *cmdline.config_file=='-')
            cmdline.config_file=NULL;  
        unbind_ports();  
        log_flush(LOG_MODE_ERROR);  
        PostMessage(hwnd, WM_INVALID_CONFIG, 0, 0);  
        WaitForSingleObject(config_ready, INFINITE);
    }
    PostMessage(hwnd, WM_VALID_CONFIG, 0, 0);
    daemon_loop();
    main_cleanup();
    _endthreadex(0);  
    return 0;
}