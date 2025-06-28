static void handle_run(HttpRequest req, HttpResponse res) {
        const char *action = get_parameter(req, "action");
        if (action) {
                if (is_readonly(req)) {
                        send_error(req, res, SC_FORBIDDEN, "You do not have sufficient privileges to access this page");
                        return;
                }
                if (IS(action, "validate")) {
                        LogInfo("The Monit http server woke up on user request\n");
                        do_wakeupcall();
                } else if (IS(action, "stop")) {
                        LogInfo("The Monit http server stopped on user request\n");
                        send_error(req, res, SC_SERVICE_UNAVAILABLE, "The Monit http server is stopped");
                        Engine_stop();
                        return;
                }
        }
        LOCK(Run.mutex)
        do_runtime(req, res);
        END_LOCK;
}