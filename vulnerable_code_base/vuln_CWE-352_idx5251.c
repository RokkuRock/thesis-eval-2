static void handle_do_action(HttpRequest req, HttpResponse res) {
        Service_T s;
        Action_Type doaction = Action_Ignored;
        const char *action = get_parameter(req, "action");
        const char *token = get_parameter(req, "token");
        if (action) {
                if (is_readonly(req)) {
                        send_error(req, res, SC_FORBIDDEN, "You do not have sufficient privileges to access this page");
                        return;
                }
                if ((doaction = Util_getAction(action)) == Action_Ignored) {
                        send_error(req, res, SC_BAD_REQUEST, "Invalid action \"%s\"", action);
                        return;
                }
                for (HttpParameter p = req->params; p; p = p->next) {
                        if (IS(p->name, "service")) {
                                s  = Util_getService(p->value);
                                if (! s) {
                                        send_error(req, res, SC_BAD_REQUEST, "There is no service named \"%s\"", p->value ? p->value : "");
                                        return;
                                }
                                s->doaction = doaction;
                                LogInfo("'%s' %s on user request\n", s->name, action);
                        }
                }
                if (token) {
                        Service_T q = NULL;
                        for (s = servicelist; s; s = s->next)
                                if (s->doaction == doaction)
                                        q = s;
                        if (q) {
                                FREE(q->token);
                                q->token = Str_dup(token);
                        }
                }
                Run.flags |= Run_ActionPending;
                do_wakeupcall();
        }
}