iolog_parse_json_object(struct json_object *object, struct eventlog *evlog)
{
    struct json_item *item;
    bool ret = false;
    debug_decl(iolog_parse_json_object, SUDO_DEBUG_UTIL);
    item = TAILQ_FIRST(&object->items);
    if (item == NULL) {
	sudo_warnx("%s", U_("missing JSON_OBJECT"));
	goto done;
    }
    if (item->type != JSON_OBJECT) {
	sudo_warnx(U_("expected JSON_OBJECT, got %d"), item->type);
	goto done;
    }
    object = &item->u.child;
    TAILQ_FOREACH(item, &object->items, entries) {
	struct iolog_json_key *key;
	if (item->name == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: missing object name", __func__);
	    goto done;
	}
	for (key = iolog_json_keys; key->name != NULL; key++) {
	    if (strcmp(item->name, key->name) == 0)
		break;
	}
	if (key->name == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: unknown key %s", __func__, item->name);
	} else if (key->type != item->type &&
		(key->type != JSON_ID || item->type != JSON_NUMBER)) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: key mismatch %s type %d, expected %d", __func__,
		item->name, item->type, key->type);
	    goto done;
	} else {
	    if (!key->setter(item, evlog)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to store %s", key->name);
		goto done;
	    }
	}
    }
    if (evlog->command != NULL && evlog->argv != NULL && evlog->argv[0] != NULL) {
	size_t len, bufsize = strlen(evlog->command) + 1;
	char *cp, *buf;
	int ac;
	for (ac = 1; evlog->argv[ac] != NULL; ac++)
	    bufsize += strlen(evlog->argv[ac]) + 1;
	if ((buf = malloc(bufsize)) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
	cp = buf;
	len = strlcpy(cp, evlog->command, bufsize);
	if (len >= bufsize)
	    sudo_fatalx(U_("internal error, %s overflow"), __func__);
	cp += len;
	bufsize -= len;
	for (ac = 1; evlog->argv[ac] != NULL; ac++) {
	    if (bufsize < 2)
		sudo_fatalx(U_("internal error, %s overflow"), __func__);
	    *cp++ = ' ';
	    bufsize--;
	    len = strlcpy(cp, evlog->argv[ac], bufsize);
	    if (len >= bufsize)
		sudo_fatalx(U_("internal error, %s overflow"), __func__);
	    cp += len;
	    bufsize -= len;
	}
	free(evlog->command);
	evlog->command = buf;
    }
    ret = true;
done:
    debug_return_bool(ret);
}