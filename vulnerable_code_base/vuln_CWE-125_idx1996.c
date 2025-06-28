fp_readl(char *s, int size, struct tok_state *tok)
{
    PyObject* bufobj;
    const char *buf;
    Py_ssize_t buflen;
    assert(size > 0);
    size--;
    if (tok->decoding_buffer) {
        bufobj = tok->decoding_buffer;
        Py_INCREF(bufobj);
    }
    else
    {
        bufobj = PyObject_CallObject(tok->decoding_readline, NULL);
        if (bufobj == NULL)
            goto error;
    }
    if (PyUnicode_CheckExact(bufobj))
    {
        buf = PyUnicode_AsUTF8AndSize(bufobj, &buflen);
        if (buf == NULL) {
            goto error;
        }
    }
    else
    {
        buf = PyByteArray_AsString(bufobj);
        if (buf == NULL) {
            goto error;
        }
        buflen = PyByteArray_GET_SIZE(bufobj);
    }
    Py_XDECREF(tok->decoding_buffer);
    if (buflen > size) {
        tok->decoding_buffer = PyByteArray_FromStringAndSize(buf+size,
                                                         buflen-size);
        if (tok->decoding_buffer == NULL)
            goto error;
        buflen = size;
    }
    else
        tok->decoding_buffer = NULL;
    memcpy(s, buf, buflen);
    s[buflen] = '\0';
    if (buflen == 0)  
        s = NULL;
    Py_DECREF(bufobj);
    return s;
error:
    Py_XDECREF(bufobj);
    return error_ret(tok);
}