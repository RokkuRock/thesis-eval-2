static void handle_action(HttpRequest req, HttpResponse res) {
        char *name = req->url;
        Service_T s = Util_getService(++name);
        if (! s) {
                send_error(req, res, SC_NOT_FOUND, "There is no service named \"%s\"", name ? name : "");
                return;
        }
        const char *action = get_parameter(req, "action");
        if (action) {
                if (is_readonly(req)) {
                        send_error(req, res, SC_FORBIDDEN, "You do not have sufficient privileges to access this page");
                        return;
                }
                Action_Type doaction = Util_getAction(action);
                if (doaction == Action_Ignored) {
                        send_error(req, res, SC_BAD_REQUEST, "Invalid action \"%s\"", action);
                        return;
                }
                s->doaction = doaction;
                const char *token = get_parameter(req, "token");
                if (token) {
                        FREE(s->token);
                        s->token = Str_dup(token);
                }
                LogInfo("'%s' %s on user request\n", s->name, action);
                Run.flags |= Run_ActionPending;  
                do_wakeupcall();
        }
        do_service(req, res, s);
}