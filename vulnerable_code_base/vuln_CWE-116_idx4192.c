match_expr(struct search_node_list *head, struct eventlog *evlog, bool last_match)
{
    struct search_node *sn;
    bool res = false, matched = last_match;
    int rc;
    debug_decl(match_expr, SUDO_DEBUG_UTIL);
    STAILQ_FOREACH(sn, head, entries) {
	switch (sn->type) {
	case ST_EXPR:
	    res = match_expr(&sn->u.expr, evlog, matched);
	    break;
	case ST_CWD:
	    if (evlog->cwd != NULL)
		res = strcmp(sn->u.cwd, evlog->cwd) == 0;
	    break;
	case ST_HOST:
	    if (evlog->submithost != NULL)
		res = strcmp(sn->u.host, evlog->submithost) == 0;
	    break;
	case ST_TTY:
	    if (evlog->ttyname != NULL)
		res = strcmp(sn->u.tty, evlog->ttyname) == 0;
	    break;
	case ST_RUNASGROUP:
	    if (evlog->rungroup != NULL)
		res = strcmp(sn->u.runas_group, evlog->rungroup) == 0;
	    break;
	case ST_RUNASUSER:
	    if (evlog->runuser != NULL)
		res = strcmp(sn->u.runas_user, evlog->runuser) == 0;
	    break;
	case ST_USER:
	    if (evlog->submituser != NULL)
		res = strcmp(sn->u.user, evlog->submituser) == 0;
	    break;
	case ST_PATTERN:
	    rc = regexec(&sn->u.cmdre, evlog->command, 0, NULL, 0);
	    if (rc && rc != REG_NOMATCH) {
		char buf[BUFSIZ];
		regerror(rc, &sn->u.cmdre, buf, sizeof(buf));
		sudo_fatalx("%s", buf);
	    }
	    res = rc == REG_NOMATCH ? 0 : 1;
	    break;
	case ST_FROMDATE:
	    res = sudo_timespeccmp(&evlog->submit_time, &sn->u.tstamp, >=);
	    break;
	case ST_TODATE:
	    res = sudo_timespeccmp(&evlog->submit_time, &sn->u.tstamp, <=);
	    break;
	default:
	    sudo_fatalx(U_("unknown search type %d"), sn->type);
	}
	if (sn->negated)
	    res = !res;
	matched = sn->or ? (res || last_match) : (res && last_match);
	last_match = matched;
    }
    debug_return_bool(matched);
}