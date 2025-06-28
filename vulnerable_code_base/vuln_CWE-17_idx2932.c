apr_status_t ap_http_filter(ap_filter_t *f, apr_bucket_brigade *b,
        ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes)
{
    core_server_config *conf;
    apr_bucket *e;
    http_ctx_t *ctx = f->ctx;
    apr_status_t rv;
    apr_off_t totalread;
    int again;
    conf = (core_server_config *)
        ap_get_module_config(f->r->server->module_config, &core_module);
    if (mode != AP_MODE_READBYTES && mode != AP_MODE_GETLINE) {
        return ap_get_brigade(f->next, b, mode, block, readbytes);
    }
    if (!ctx) {
        const char *tenc, *lenp;
        f->ctx = ctx = apr_pcalloc(f->r->pool, sizeof(*ctx));
        ctx->state = BODY_NONE;
        if (!f->r->proxyreq) {
            ctx->limit = ap_get_limit_req_body(f->r);
        }
        else {
            ctx->limit = 0;
        }
        tenc = apr_table_get(f->r->headers_in, "Transfer-Encoding");
        lenp = apr_table_get(f->r->headers_in, "Content-Length");
        if (tenc) {
            if (strcasecmp(tenc, "chunked") == 0  
                    || ap_find_last_token(f->r->pool, tenc, "chunked")) {
                ctx->state = BODY_CHUNK;
            }
            else if (f->r->proxyreq == PROXYREQ_RESPONSE) {
                ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, f->r, APLOGNO(02555)
                              "Unknown Transfer-Encoding: %s;"
                              " using read-until-close", tenc);
                tenc = NULL;
            }
            else {
                ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, f->r, APLOGNO(01585)
                              "Unknown Transfer-Encoding: %s", tenc);
                return APR_EGENERAL;
            }
            lenp = NULL;
        }
        if (lenp) {
            char *endstr;
            ctx->state = BODY_LENGTH;
            if (apr_strtoff(&ctx->remaining, lenp, &endstr, 10)
                    || endstr == lenp || *endstr || ctx->remaining < 0) {
                ctx->remaining = 0;
                ap_log_rerror(
                        APLOG_MARK, APLOG_INFO, 0, f->r, APLOGNO(01587)
                        "Invalid Content-Length");
                return APR_ENOSPC;
            }
            if (ctx->limit && ctx->limit < ctx->remaining) {
                ap_log_rerror(
                        APLOG_MARK, APLOG_INFO, 0, f->r, APLOGNO(01588)
                        "Requested content-length of %" APR_OFF_T_FMT
                        " is larger than the configured limit"
                        " of %" APR_OFF_T_FMT, ctx->remaining, ctx->limit);
                return APR_ENOSPC;
            }
        }
        if (ctx->state == BODY_NONE && f->r->proxyreq != PROXYREQ_RESPONSE) {
            e = apr_bucket_eos_create(f->c->bucket_alloc);
            APR_BRIGADE_INSERT_TAIL(b, e);
            ctx->eos_sent = 1;
            return APR_SUCCESS;
        }
        if ((ctx->state == BODY_CHUNK
                || (ctx->state == BODY_LENGTH && ctx->remaining > 0))
                && f->r->expecting_100 && f->r->proto_num >= HTTP_VERSION(1,1)
                && !(f->r->eos_sent || f->r->bytes_sent)) {
            if (!ap_is_HTTP_SUCCESS(f->r->status)) {
                ctx->state = BODY_NONE;
                ctx->eos_sent = 1;
            }
            else {
                char *tmp;
                int len;
                apr_bucket_brigade *bb;
                bb = apr_brigade_create(f->r->pool, f->c->bucket_alloc);
                f->r->expecting_100 = 0;
                tmp = apr_pstrcat(f->r->pool, AP_SERVER_PROTOCOL " ",
                        ap_get_status_line(HTTP_CONTINUE), CRLF CRLF, NULL);
                len = strlen(tmp);
                ap_xlate_proto_to_ascii(tmp, len);
                e = apr_bucket_pool_create(tmp, len, f->r->pool,
                        f->c->bucket_alloc);
                APR_BRIGADE_INSERT_HEAD(bb, e);
                e = apr_bucket_flush_create(f->c->bucket_alloc);
                APR_BRIGADE_INSERT_TAIL(bb, e);
                rv = ap_pass_brigade(f->c->output_filters, bb);
                if (rv != APR_SUCCESS) {
                    return AP_FILTER_ERROR;
                }
            }
        }
    }
    if (ctx->eos_sent) {
        e = apr_bucket_eos_create(f->c->bucket_alloc);
        APR_BRIGADE_INSERT_TAIL(b, e);
        return APR_SUCCESS;
    }
    do {
        apr_brigade_cleanup(b);
        again = 0;  
        switch (ctx->state) {
        case BODY_CHUNK:
        case BODY_CHUNK_PART:
        case BODY_CHUNK_EXT:
        case BODY_CHUNK_END: {
            rv = ap_get_brigade(f->next, b, AP_MODE_GETLINE, block, 0);
            if (block == APR_NONBLOCK_READ
                    && ((rv == APR_SUCCESS && APR_BRIGADE_EMPTY(b))
                            || (APR_STATUS_IS_EAGAIN(rv)))) {
                return APR_EAGAIN;
            }
            if (rv == APR_EOF) {
                return APR_INCOMPLETE;
            }
            if (rv != APR_SUCCESS) {
                return rv;
            }
            e = APR_BRIGADE_FIRST(b);
            while (e != APR_BRIGADE_SENTINEL(b)) {
                const char *buffer;
                apr_size_t len;
                if (!APR_BUCKET_IS_METADATA(e)) {
                    rv = apr_bucket_read(e, &buffer, &len, APR_BLOCK_READ);
                    if (rv == APR_SUCCESS) {
                        rv = parse_chunk_size(ctx, buffer, len,
                                f->r->server->limit_req_fieldsize);
                    }
                    if (rv != APR_SUCCESS) {
                        ap_log_rerror(
                                APLOG_MARK, APLOG_INFO, rv, f->r, APLOGNO(01590) "Error reading chunk %s ", (APR_ENOSPC == rv) ? "(overflow)" : "");
                        return rv;
                    }
                }
                apr_bucket_delete(e);
                e = APR_BRIGADE_FIRST(b);
            }
            again = 1;  
            if (ctx->state == BODY_CHUNK_TRAILER) {
                int merge_trailers =
                    conf->merge_trailers == AP_MERGE_TRAILERS_ENABLE;
                return read_chunked_trailers(ctx, f, b, merge_trailers);
            }
            break;
        }
        case BODY_NONE:
        case BODY_LENGTH:
        case BODY_CHUNK_DATA: {
            if (ctx->state != BODY_NONE && ctx->remaining < readbytes) {
                readbytes = ctx->remaining;
            }
            if (readbytes > 0) {
                rv = ap_get_brigade(f->next, b, mode, block, readbytes);
                if (block == APR_NONBLOCK_READ
                        && ((rv == APR_SUCCESS && APR_BRIGADE_EMPTY(b))
                                || (APR_STATUS_IS_EAGAIN(rv)))) {
                    return APR_EAGAIN;
                }
                if (rv == APR_EOF && ctx->state != BODY_NONE
                        && ctx->remaining > 0) {
                    return APR_INCOMPLETE;
                }
                if (rv != APR_SUCCESS) {
                    return rv;
                }
                apr_brigade_length(b, 0, &totalread);
                AP_DEBUG_ASSERT(totalread >= 0);
                if (ctx->state != BODY_NONE) {
                    ctx->remaining -= totalread;
                    if (ctx->remaining > 0) {
                        e = APR_BRIGADE_LAST(b);
                        if (APR_BUCKET_IS_EOS(e)) {
                            apr_bucket_delete(e);
                            return APR_INCOMPLETE;
                        }
                    }
                    else if (ctx->state == BODY_CHUNK_DATA) {
                        ctx->state = BODY_CHUNK_END;
                        ctx->chunk_used = 0;
                    }
                }
            }
            if (ctx->state == BODY_LENGTH && ctx->remaining == 0) {
                e = apr_bucket_eos_create(f->c->bucket_alloc);
                APR_BRIGADE_INSERT_TAIL(b, e);
                ctx->eos_sent = 1;
            }
            if (ctx->limit) {
                ctx->limit_used += totalread;
                if (ctx->limit < ctx->limit_used) {
                    ap_log_rerror(
                            APLOG_MARK, APLOG_INFO, 0, f->r, APLOGNO(01591) "Read content-length of %" APR_OFF_T_FMT " is larger than the configured limit"
                            " of %" APR_OFF_T_FMT, ctx->limit_used, ctx->limit);
                    return APR_ENOSPC;
                }
            }
            break;
        }
        case BODY_CHUNK_TRAILER: {
            rv = ap_get_brigade(f->next, b, mode, block, readbytes);
            if (block == APR_NONBLOCK_READ
                    && ((rv == APR_SUCCESS && APR_BRIGADE_EMPTY(b))
                            || (APR_STATUS_IS_EAGAIN(rv)))) {
                return APR_EAGAIN;
            }
            if (rv != APR_SUCCESS) {
                return rv;
            }
            break;
        }
        default: {
            break;
        }
        }
    } while (again);
    return APR_SUCCESS;
}