void set_header(HttpResponse res, const char *name, const char *value) {
        HttpHeader h = NULL;
        ASSERT(res);
        ASSERT(name);
        NEW(h);
        h->name = Str_dup(name);
        h->value = Str_dup(value);
        if (res->headers) {
                HttpHeader n, p;
                for (n = p = res->headers; p; n = p, p = p->next) {
                        if (IS(p->name, name)) {
                                FREE(p->value);
                                p->value = Str_dup(value);
                                destroy_entry(h);
                                return;
                        }
                }
                n->next = h;
        } else {
                res->headers = h;
        }
}