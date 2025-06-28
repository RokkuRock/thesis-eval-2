decode_bytes_with_escapes(struct compiling *c, const node *n, const char *s,
                          size_t len)
{
    return PyBytes_DecodeEscape(s, len, NULL, 0, NULL);
}