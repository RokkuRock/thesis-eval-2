NOEXPORT char *parse_global_option(CMD cmd, char *opt, char *arg) {
    void *tmp;
    if(cmd==CMD_PRINT_DEFAULTS || cmd==CMD_PRINT_HELP) {
        s_log(LOG_NOTICE, " ");
        s_log(LOG_NOTICE, "Global options:");
    }
#ifdef HAVE_CHROOT
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.chroot_dir=NULL;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        tmp=global_options.chroot_dir;
        global_options.chroot_dir=NULL;
        str_free(tmp);
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "chroot"))
            break;
        new_global_options.chroot_dir=str_dup(arg);
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = directory to chroot stunnel process", "chroot");
        break;
    }
#endif  
#ifndef OPENSSL_NO_COMP
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.compression=COMP_NONE;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "compression"))
            break;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if(OpenSSL_version_num()<0x00908051L)  
            return "Compression unsupported due to a memory leak";
#endif  
        if(!strcasecmp(arg, "deflate"))
            new_global_options.compression=COMP_DEFLATE;
        else if(!strcasecmp(arg, "zlib"))
            new_global_options.compression=COMP_ZLIB;
        else
            return "Specified compression type is not available";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = compression type",
            "compression");
        break;
    }
#endif  
    switch(cmd) {
    case CMD_SET_DEFAULTS:
#ifdef EGD_SOCKET
        new_global_options.egd_sock=EGD_SOCKET;
#else
        new_global_options.egd_sock=NULL;
#endif
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        tmp=global_options.egd_sock;
        global_options.egd_sock=NULL;
        str_free(tmp);
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "EGD"))
            break;
        new_global_options.egd_sock=str_dup(arg);
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
#ifdef EGD_SOCKET
        s_log(LOG_NOTICE, "%-22s = %s", "EGD", EGD_SOCKET);
#endif
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = path to Entropy Gathering Daemon socket", "EGD");
        break;
    }
#ifndef OPENSSL_NO_ENGINE
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        engine_reset_list();
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "engine"))
            break;
        if(!strcasecmp(arg, "auto"))
            return engine_auto();
        else
            return engine_open(arg);
    case CMD_INITIALIZE:
        engine_init();
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = auto|engine_id",
            "engine");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "engineCtrl"))
            break;
        {
            char *tmp_str=strchr(arg, ':');
            if(tmp_str)
                *tmp_str++='\0';
            return engine_ctrl(arg, tmp_str);
        }
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = cmd[:arg]",
            "engineCtrl");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "engineDefault"))
            break;
        return engine_default(arg);
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = TASK_LIST",
            "engineDefault");
        break;
    }
#endif  
    switch(cmd) {
    case CMD_SET_DEFAULTS:
#ifdef USE_FIPS
        new_global_options.option.fips=0;
#endif  
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "fips"))
            break;
#ifdef USE_FIPS
        if(!strcasecmp(arg, "yes"))
            new_global_options.option.fips=1;
        else if(!strcasecmp(arg, "no"))
            new_global_options.option.fips=0;
        else
            return "The argument needs to be either 'yes' or 'no'";
#else
        if(strcasecmp(arg, "no"))
            return "FIPS support is not available";
#endif  
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
#ifdef USE_FIPS
        s_log(LOG_NOTICE, "%-22s = yes|no FIPS 140-2 mode",
            "fips");
#endif  
        break;
    }
#ifndef USE_WIN32
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.option.foreground=0;
        new_global_options.option.log_stderr=0;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "foreground"))
            break;
        if(!strcasecmp(arg, "yes")) {
            new_global_options.option.foreground=1;
            new_global_options.option.log_stderr=1;
        } else if(!strcasecmp(arg, "quiet")) {
            new_global_options.option.foreground=1;
            new_global_options.option.log_stderr=0;
        } else if(!strcasecmp(arg, "no")) {
            new_global_options.option.foreground=0;
            new_global_options.option.log_stderr=0;
        } else
            return "The argument needs to be either 'yes', 'quiet' or 'no'";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = yes|quiet|no foreground mode (don't fork, log to stderr)",
            "foreground");
        break;
    }
#endif
#ifdef ICON_IMAGE
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.icon[ICON_ACTIVE]=load_icon_default(ICON_ACTIVE);
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "iconActive"))
            break;
        if(!(new_global_options.icon[ICON_ACTIVE]=load_icon_file(arg)))
            return "Failed to load the specified icon";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = icon when connections are established", "iconActive");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.icon[ICON_ERROR]=load_icon_default(ICON_ERROR);
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "iconError"))
            break;
        if(!(new_global_options.icon[ICON_ERROR]=load_icon_file(arg)))
            return "Failed to load the specified icon";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = icon for invalid configuration file", "iconError");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.icon[ICON_IDLE]=load_icon_default(ICON_IDLE);
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "iconIdle"))
            break;
        if(!(new_global_options.icon[ICON_IDLE]=load_icon_file(arg)))
            return "Failed to load the specified icon";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = icon when no connections were established", "iconIdle");
        break;
    }
#endif  
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.log_file_mode=FILE_MODE_APPEND;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "log"))
            break;
        if(!strcasecmp(arg, "append"))
            new_global_options.log_file_mode=FILE_MODE_APPEND;
        else if(!strcasecmp(arg, "overwrite"))
            new_global_options.log_file_mode=FILE_MODE_OVERWRITE;
        else
            return "The argument needs to be either 'append' or 'overwrite'";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = append|overwrite log file",
            "log");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.output_file=NULL;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        tmp=global_options.output_file;
        global_options.output_file=NULL;
        str_free(tmp);
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "output"))
            break;
        new_global_options.output_file=str_dup(arg);
        return NULL;  
    case CMD_INITIALIZE:
#ifndef USE_WIN32
        if(!new_global_options.option.foreground   &&
                new_global_options.output_file   &&
                new_global_options.output_file[0]!='/'  )
            return "Log file must include full path name";
#endif
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = file to append log messages", "output");
        break;
    }
#ifndef USE_WIN32
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.pidfile=NULL;  
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        tmp=global_options.pidfile;
        global_options.pidfile=NULL;
        str_free(tmp);
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "pid"))
            break;
        if(arg[0])  
            new_global_options.pidfile=str_dup(arg);
        else
            new_global_options.pidfile=NULL;  
        return NULL;  
    case CMD_INITIALIZE:
        if(!new_global_options.option.foreground   &&
                new_global_options.pidfile   &&
                new_global_options.pidfile[0]!='/'  )
            return "Pid file must include full path name";
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = pid file", "pid");
        break;
    }
#endif
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.random_bytes=RANDOM_BYTES;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "RNDbytes"))
            break;
        {
            char *tmp_str;
            new_global_options.random_bytes=(long)strtol(arg, &tmp_str, 10);
            if(tmp_str==arg || *tmp_str)  
                return "Illegal number of bytes to read from random seed files";
        }
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        s_log(LOG_NOTICE, "%-22s = %d", "RNDbytes", RANDOM_BYTES);
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = bytes to read from random seed files", "RNDbytes");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
#ifdef RANDOM_FILE
        new_global_options.rand_file=str_dup(RANDOM_FILE);
#else
        new_global_options.rand_file=NULL;
#endif
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        tmp=global_options.rand_file;
        global_options.rand_file=NULL;
        str_free(tmp);
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "RNDfile"))
            break;
        new_global_options.rand_file=str_dup(arg);
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
#ifdef RANDOM_FILE
        s_log(LOG_NOTICE, "%-22s = %s", "RNDfile", RANDOM_FILE);
#endif
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = path to file with random seed data", "RNDfile");
        break;
    }
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.option.rand_write=1;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "RNDoverwrite"))
            break;
        if(!strcasecmp(arg, "yes"))
            new_global_options.option.rand_write=1;
        else if(!strcasecmp(arg, "no"))
            new_global_options.option.rand_write=0;
        else
            return "The argument needs to be either 'yes' or 'no'";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        s_log(LOG_NOTICE, "%-22s = yes", "RNDoverwrite");
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = yes|no overwrite seed datafiles with new random data",
            "RNDoverwrite");
        break;
    }
#ifndef USE_WIN32
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.option.log_syslog=1;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "syslog"))
            break;
        if(!strcasecmp(arg, "yes"))
            new_global_options.option.log_syslog=1;
        else if(!strcasecmp(arg, "no"))
            new_global_options.option.log_syslog=0;
        else
            return "The argument needs to be either 'yes' or 'no'";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = yes|no send logging messages to syslog",
            "syslog");
        break;
    }
#endif
#ifdef USE_WIN32
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        new_global_options.option.taskbar=1;
        break;
    case CMD_SET_COPY:  
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        if(strcasecmp(opt, "taskbar"))
            break;
        if(!strcasecmp(arg, "yes"))
            new_global_options.option.taskbar=1;
        else if(!strcasecmp(arg, "no"))
            new_global_options.option.taskbar=0;
        else
            return "The argument needs to be either 'yes' or 'no'";
        return NULL;  
    case CMD_INITIALIZE:
        break;
    case CMD_PRINT_DEFAULTS:
        s_log(LOG_NOTICE, "%-22s = yes", "taskbar");
        break;
    case CMD_PRINT_HELP:
        s_log(LOG_NOTICE, "%-22s = yes|no enable the taskbar icon", "taskbar");
        break;
    }
#endif
    switch(cmd) {
    case CMD_SET_DEFAULTS:
        break;
    case CMD_SET_COPY:
        break;
    case CMD_FREE:
        break;
    case CMD_SET_VALUE:
        return option_not_found;
    case CMD_INITIALIZE:
        if(ssl_configure(&new_global_options))  
            return "Failed to initialize TLS";
    case CMD_PRINT_DEFAULTS:
        break;
    case CMD_PRINT_HELP:
        break;
    }
    return NULL;  
}