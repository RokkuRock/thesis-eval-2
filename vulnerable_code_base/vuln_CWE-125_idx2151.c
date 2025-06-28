decode_unicode_with_escapes(struct compiling *c, const node *n, const char *s,
                            size_t len)
{
    PyObject *u;
    char *buf;
    char *p;
    const char *end;
    if (len > SIZE_MAX / 6)
        return NULL;
    u = PyBytes_FromStringAndSize((char *)NULL, len * 6);
    if (u == NULL)
        return NULL;
    p = buf = PyBytes_AsString(u);
    end = s + len;
    while (s < end) {
        if (*s == '\\') {
            *p++ = *s++;
            if (*s & 0x80) {
                strcpy(p, "u005c");
                p += 5;
            }
        }
        if (*s & 0x80) {  
            PyObject *w;
            int kind;
            void *data;
            Py_ssize_t len, i;
            w = decode_utf8(c, &s, end);
            if (w == NULL) {
                Py_DECREF(u);
                return NULL;
            }
            kind = PyUnicode_KIND(w);
            data = PyUnicode_DATA(w);
            len = PyUnicode_GET_LENGTH(w);
            for (i = 0; i < len; i++) {
                Py_UCS4 chr = PyUnicode_READ(kind, data, i);
                sprintf(p, "\\U%08x", chr);
                p += 10;
            }
            assert(p - buf <= Py_SIZE(u));
            Py_DECREF(w);
        } else {
            *p++ = *s++;
        }
    }
    len = p - buf;
    s = buf;
    return PyUnicode_DecodeUnicodeEscape(s, len, NULL);
}