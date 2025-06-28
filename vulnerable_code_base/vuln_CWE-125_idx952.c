decoding_feof(struct tok_state *tok)
{
    if (tok->decoding_state != STATE_NORMAL) {
        return feof(tok->fp);
    } else {
        PyObject* buf = tok->decoding_buffer;
        if (buf == NULL) {
            buf = PyObject_CallObject(tok->decoding_readline, NULL);
            if (buf == NULL) {
                error_ret(tok);
                return 1;
            } else {
                tok->decoding_buffer = buf;
            }
        }
        return PyObject_Length(buf) == 0;
    }
}