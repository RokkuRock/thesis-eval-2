static void doPost(HttpRequest req, HttpResponse res) {
        set_content_type(res, "text/html");
        if (ACTION(RUN))
                handle_run(req, res);
        else if (ACTION(STATUS))
                print_status(req, res, 1);
        else if (ACTION(STATUS2))
                print_status(req, res, 2);
        else if (ACTION(SUMMARY))
                print_summary(req, res);
        else if (ACTION(REPORT))
                _printReport(req, res);
        else if (ACTION(DOACTION))
                handle_do_action(req, res);
        else
                handle_action(req, res);
}