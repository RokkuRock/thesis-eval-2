static apr_status_t parse_chunk_size(http_ctx_t *ctx, const char *buffer,
        apr_size_t len, int linelimit)
{
    apr_size_t i = 0;
    while (i < len) {
        char c = buffer[i];
        ap_xlate_proto_from_ascii(&c, 1);
        if (ctx->state == BODY_CHUNK_END) {
            if (c == LF) {
                ctx->state = BODY_CHUNK;
            }
            i++;
            continue;
        }
        if (ctx->state == BODY_CHUNK) {
            if (!apr_isxdigit(c)) {
                return APR_EGENERAL;
            }
            else {
                ctx->state = BODY_CHUNK_PART;
            }
            ctx->remaining = 0;
            ctx->chunkbits = sizeof(long) * 8;
            ctx->chunk_used = 0;
        }
        if (c == ';' || c == CR) {
            ctx->state = BODY_CHUNK_EXT;
        }
        else if (c == LF) {
            if (ctx->remaining) {
                ctx->state = BODY_CHUNK_DATA;
            }
            else {
                ctx->state = BODY_CHUNK_TRAILER;
            }
        }
        else if (ctx->state != BODY_CHUNK_EXT) {
            int xvalue = 0;
            if (!ctx->remaining && c == '0') {
                i++;
                continue;
            }
            if (c >= '0' && c <= '9') {
                xvalue = c - '0';
            }
            else if (c >= 'A' && c <= 'F') {
                xvalue = c - 'A' + 0xa;
            }
            else if (c >= 'a' && c <= 'f') {
                xvalue = c - 'a' + 0xa;
            }
            else {
                return APR_EGENERAL;
            }
            ctx->remaining = (ctx->remaining << 4) | xvalue;
            ctx->chunkbits -= 4;
            if (ctx->chunkbits <= 0 || ctx->remaining < 0) {
                return APR_ENOSPC;
            }
        }
        i++;
    }
    ctx->chunk_used += len;
    if (ctx->chunk_used < 0 || ctx->chunk_used > linelimit) {
        return APR_ENOSPC;
    }
    return APR_SUCCESS;
}