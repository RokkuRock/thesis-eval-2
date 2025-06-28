list_session(char *log_dir, regex_t *re, const char *user, const char *tty)
{
    char idbuf[7], *idstr, *cp;
    struct eventlog *evlog = NULL;
    const char *timestr;
    int ret = -1;
    debug_decl(list_session, SUDO_DEBUG_UTIL);
    if ((evlog = iolog_parse_loginfo(-1, log_dir)) == NULL)
	goto done;
    if (evlog->command == NULL || evlog->submituser == NULL ||
	    evlog->runuser == NULL) {
	goto done;
    }
    if (!STAILQ_EMPTY(&search_expr) && !match_expr(&search_expr, evlog, true))
	goto done;
    cp = log_dir + strlen(session_dir) + 1;
    if (IS_IDLOG(cp)) {
	idbuf[0] = cp[0];
	idbuf[1] = cp[1];
	idbuf[2] = cp[3];
	idbuf[3] = cp[4];
	idbuf[4] = cp[6];
	idbuf[5] = cp[7];
	idbuf[6] = '\0';
	idstr = idbuf;
    } else {
	idstr = cp;
    }
    timestr = get_timestr(evlog->submit_time.tv_sec, 1);
    printf("%s : %s : ", timestr ? timestr : "invalid date", evlog->submituser);
    if (evlog->submithost != NULL)
	printf("HOST=%s ; ", evlog->submithost);
    if (evlog->ttyname != NULL)
	printf("TTY=%s ; ", evlog->ttyname);
    if (evlog->runchroot != NULL)
	printf("CHROOT=%s ; ", evlog->runchroot);
    if (evlog->runcwd != NULL || evlog->cwd != NULL)
	printf("CWD=%s ; ", evlog->runcwd ? evlog->runcwd : evlog->cwd);
    printf("USER=%s ; ", evlog->runuser);
    if (evlog->rungroup != NULL)
	printf("GROUP=%s ; ", evlog->rungroup);
    printf("TSID=%s ; COMMAND=%s\n", idstr, evlog->command);
    ret = 0;
done:
    eventlog_free(evlog);
    debug_return_int(ret);
}