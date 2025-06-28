void set_content_type(HttpResponse res, const char *mime) {
        set_header(res, "Content-Type", mime);
}