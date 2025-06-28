new_logline(int event_type, int flags, struct eventlog_args *args,
    const struct eventlog *evlog)
{
    const struct eventlog_config *evl_conf = eventlog_getconf();
    char *line = NULL, *evstr = NULL;
    const char *iolog_file;
    const char *tty, *tsid = NULL;
    char exit_str[(((sizeof(int) * 8) + 2) / 3) + 2];
    char sessid[7], offsetstr[64] = "";
    size_t len = 0;
    int i;
    debug_decl(new_logline, SUDO_DEBUG_UTIL);
    if (ISSET(flags, EVLOG_RAW) || evlog == NULL) {
	if (args->reason != NULL) {
	    if (args->errstr != NULL) {
		if (asprintf(&line, "%s: %s", args->reason, args->errstr) == -1)
		    goto oom;
	    } else {
		if ((line = strdup(args->reason)) == NULL)
		    goto oom;
	    }
	}
	debug_return_str(line);
    }
    iolog_file = evlog->iolog_file;
    if (iolog_file != NULL) {
	if (IS_SESSID(iolog_file)) {
	    sessid[0] = iolog_file[0];
	    sessid[1] = iolog_file[1];
	    sessid[2] = iolog_file[3];
	    sessid[3] = iolog_file[4];
	    sessid[4] = iolog_file[6];
	    sessid[5] = iolog_file[7];
	    sessid[6] = '\0';
	    tsid = sessid;
	} else {
	    tsid = iolog_file;
	}
	if (sudo_timespecisset(&evlog->iolog_offset)) {
	    if (evlog->iolog_offset.tv_nsec > 10000000) {
		(void)snprintf(offsetstr, sizeof(offsetstr), "@%lld.%02ld",
		    (long long)evlog->iolog_offset.tv_sec,
		    evlog->iolog_offset.tv_nsec / 10000000);
	    } else if (evlog->iolog_offset.tv_sec != 0) {
		(void)snprintf(offsetstr, sizeof(offsetstr), "@%lld",
		    (long long)evlog->iolog_offset.tv_sec);
	    }
	}
    }
    if ((tty = evlog->ttyname) != NULL) {
	if (strncmp(tty, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
	    tty += sizeof(_PATH_DEV) - 1;
    }
    if (args->reason != NULL)
	len += strlen(args->reason) + 3;
    if (args->errstr != NULL)
	len += strlen(args->errstr) + 3;
    if (evlog->submithost != NULL && !evl_conf->omit_hostname)
	len += sizeof(LL_HOST_STR) + 2 + strlen(evlog->submithost);
    if (tty != NULL)
	len += sizeof(LL_TTY_STR) + 2 + strlen(tty);
    if (evlog->runchroot != NULL)
	len += sizeof(LL_CHROOT_STR) + 2 + strlen(evlog->runchroot);
    if (evlog->runcwd != NULL)
	len += sizeof(LL_CWD_STR) + 2 + strlen(evlog->runcwd);
    if (evlog->runuser != NULL)
	len += sizeof(LL_USER_STR) + 2 + strlen(evlog->runuser);
    if (evlog->rungroup != NULL)
	len += sizeof(LL_GROUP_STR) + 2 + strlen(evlog->rungroup);
    if (tsid != NULL) {
	len += sizeof(LL_TSID_STR) + 2 + strlen(tsid) + strlen(offsetstr);
    }
    if (evlog->env_add != NULL) {
	size_t evlen = 0;
	char * const *ep;
	for (ep = evlog->env_add; *ep != NULL; ep++)
	    evlen += strlen(*ep) + 1;
	if (evlen != 0) {
	    if ((evstr = malloc(evlen)) == NULL)
		goto oom;
	    ep = evlog->env_add;
	    if (strlcpy(evstr, *ep, evlen) >= evlen)
		goto toobig;
	    while (*++ep != NULL) {
		if (strlcat(evstr, " ", evlen) >= evlen ||
		    strlcat(evstr, *ep, evlen) >= evlen)
		    goto toobig;
	    }
	    len += sizeof(LL_ENV_STR) + 2 + evlen;
	}
    }
    if (evlog->command != NULL) {
	len += sizeof(LL_CMND_STR) - 1 + strlen(evlog->command);
	if (evlog->argv != NULL && evlog->argv[0] != NULL) {
	    for (i = 1; evlog->argv[i] != NULL; i++)
		len += strlen(evlog->argv[i]) + 1;
	}
	if (event_type == EVLOG_EXIT) {
	    if (evlog->signal_name != NULL)
		len += sizeof(LL_SIGNAL_STR) + 2 + strlen(evlog->signal_name);
	    if (evlog->exit_value != -1) {
		(void)snprintf(exit_str, sizeof(exit_str), "%d", evlog->exit_value);
		len += sizeof(LL_EXIT_STR) + 2 + strlen(exit_str);
	    }
	}
    }
    if ((line = malloc(++len)) == NULL)
	goto oom;
    line[0] = '\0';
    if (args->reason != NULL) {
	if (strlcat(line, args->reason, len) >= len ||
	    strlcat(line, args->errstr ? " : " : " ; ", len) >= len)
	    goto toobig;
    }
    if (args->errstr != NULL) {
	if (strlcat(line, args->errstr, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evlog->submithost != NULL && !evl_conf->omit_hostname) {
	if (strlcat(line, LL_HOST_STR, len) >= len ||
	    strlcat(line, evlog->submithost, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (tty != NULL) {
	if (strlcat(line, LL_TTY_STR, len) >= len ||
	    strlcat(line, tty, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evlog->runchroot != NULL) {
	if (strlcat(line, LL_CHROOT_STR, len) >= len ||
	    strlcat(line, evlog->runchroot, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evlog->runcwd != NULL) {
	if (strlcat(line, LL_CWD_STR, len) >= len ||
	    strlcat(line, evlog->runcwd, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evlog->runuser != NULL) {
	if (strlcat(line, LL_USER_STR, len) >= len ||
	    strlcat(line, evlog->runuser, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evlog->rungroup != NULL) {
	if (strlcat(line, LL_GROUP_STR, len) >= len ||
	    strlcat(line, evlog->rungroup, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (tsid != NULL) {
	if (strlcat(line, LL_TSID_STR, len) >= len ||
	    strlcat(line, tsid, len) >= len ||
	    strlcat(line, offsetstr, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
    }
    if (evstr != NULL) {
	if (strlcat(line, LL_ENV_STR, len) >= len ||
	    strlcat(line, evstr, len) >= len ||
	    strlcat(line, " ; ", len) >= len)
	    goto toobig;
	free(evstr);
	evstr = NULL;
    }
    if (evlog->command != NULL) {
	if (strlcat(line, LL_CMND_STR, len) >= len)
	    goto toobig;
	if (strlcat(line, evlog->command, len) >= len)
	    goto toobig;
	if (evlog->argv != NULL && evlog->argv[0] != NULL) {
	    for (i = 1; evlog->argv[i] != NULL; i++) {
		if (strlcat(line, " ", len) >= len ||
		    strlcat(line, evlog->argv[i], len) >= len)
		    goto toobig;
	    }
	}
	if (event_type == EVLOG_EXIT) {
	    if (evlog->signal_name != NULL) {
		if (strlcat(line, " ; ", len) >= len ||
		    strlcat(line, LL_SIGNAL_STR, len) >= len ||
		    strlcat(line, evlog->signal_name, len) >= len)
		    goto toobig;
	    }
	    if (evlog->exit_value != -1) {
		if (strlcat(line, " ; ", len) >= len ||
		    strlcat(line, LL_EXIT_STR, len) >= len ||
		    strlcat(line, exit_str, len) >= len)
		    goto toobig;
	    }
	}
    }
    debug_return_str(line);
oom:
    free(evstr);
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_str(NULL);
toobig:
    free(evstr);
    free(line);
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    debug_return_str(NULL);
}