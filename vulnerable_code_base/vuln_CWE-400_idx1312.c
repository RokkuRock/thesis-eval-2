void out_string(conn *c, const char *str) {
    size_t len;
    mc_resp *resp = c->resp;
    assert(c != NULL);
    resp_reset(resp);
    if (c->noreply) {
        resp->skip = true;
        if (settings.verbose > 1)
            fprintf(stderr, ">%d NOREPLY %s\n", c->sfd, str);
        conn_set_state(c, conn_new_cmd);
        return;
    }
    if (settings.verbose > 1)
        fprintf(stderr, ">%d %s\n", c->sfd, str);
    len = strlen(str);
    if ((len + 2) > WRITE_BUFFER_SIZE) {
        str = "SERVER_ERROR output line too long";
        len = strlen(str);
    }
    memcpy(resp->wbuf, str, len);
    memcpy(resp->wbuf + len, "\r\n", 2);
    resp_add_iov(resp, resp->wbuf, len + 2);
    conn_set_state(c, conn_new_cmd);
    return;
}