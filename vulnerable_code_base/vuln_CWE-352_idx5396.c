static void print_buttons(HttpRequest req, HttpResponse res, Service_T s) {
        if (is_readonly(req)) {
                return;
        }
        StringBuffer_append(res->outputbuffer, "<table id='buttons'><tr>");
        if (s->start)
                StringBuffer_append(res->outputbuffer,
                                    "<td><form method=POST action=%s>"
                                    "<input type=hidden value='start' name=action>"
                                    "<input type=submit value='Start service'></form></td>", s->name);
        if (s->stop)
                StringBuffer_append(res->outputbuffer,
                                    "<td><form method=POST action=%s>"
                                    "<input type=hidden value='stop' name=action>"
                                    "<input type=submit value='Stop service'></form></td>", s->name);
        if ((s->start && s->stop) || s->restart)
                StringBuffer_append(res->outputbuffer,
                                    "<td><form method=POST action=%s>"
                                    "<input type=hidden value='restart' name=action>"
                                    "<input type=submit value='Restart service'></form></td>", s->name);
        StringBuffer_append(res->outputbuffer,
                            "<td><form method=POST action=%s>"
                            "<input type=hidden value='%s' name=action>"
                            "<input type=submit value='%s'></form></td></tr></table>",
                            s->name,
                            s->monitor ? "unmonitor" : "monitor",
                            s->monitor ? "Disable monitoring" : "Enable monitoring");
}