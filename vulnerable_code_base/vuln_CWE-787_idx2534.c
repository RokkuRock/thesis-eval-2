static int ntlm_decode_u16l_str_hdr(struct ntlm_ctx *ctx,
                                    struct wire_field_hdr *str_hdr,
                                    struct ntlm_buffer *buffer,
                                    size_t payload_offs, char **str)
{
    char *in, *out = NULL;
    uint16_t str_len;
    uint32_t str_offs;
    size_t outlen;
    int ret = 0;
    str_len = le16toh(str_hdr->len);
    if (str_len == 0) goto done;
    str_offs = le32toh(str_hdr->offset);
    if ((str_offs < payload_offs) ||
        (str_offs > buffer->length) ||
        (UINT32_MAX - str_offs < str_len) ||
        (str_offs + str_len > buffer->length)) {
        return ERR_DECODE;
    }
    in = (char *)&buffer->data[str_offs];
    out = malloc(str_len * 2 + 1);
    if (!out) return ENOMEM;
    ret = ntlm_str_convert(ctx->to_oem, in, out, str_len, &outlen);
    out[outlen] = '\0';
done:
    if (ret) {
        safefree(out);
    }
    *str = out;
    return ret;
}