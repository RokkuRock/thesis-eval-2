Ta3Tokenizer_FindEncodingFilename(int fd, PyObject *filename)
{
    struct tok_state *tok;
    FILE *fp;
    char *p_start =NULL , *p_end =NULL , *encoding = NULL;
#ifndef PGEN
#if PY_MINOR_VERSION >= 4
    fd = _Py_dup(fd);
#endif
#else
    fd = dup(fd);
#endif
    if (fd < 0) {
        return NULL;
    }
    fp = fdopen(fd, "r");
    if (fp == NULL) {
        return NULL;
    }
    tok = Ta3Tokenizer_FromFile(fp, NULL, NULL, NULL);
    if (tok == NULL) {
        fclose(fp);
        return NULL;
    }
#ifndef PGEN
    if (filename != NULL) {
        Py_INCREF(filename);
        tok->filename = filename;
    }
    else {
        tok->filename = PyUnicode_FromString("<string>");
        if (tok->filename == NULL) {
            fclose(fp);
            Ta3Tokenizer_Free(tok);
            return encoding;
        }
    }
#endif
    while (tok->lineno < 2 && tok->done == E_OK) {
        Ta3Tokenizer_Get(tok, &p_start, &p_end);
    }
    fclose(fp);
    if (tok->encoding) {
        encoding = (char *)PyMem_MALLOC(strlen(tok->encoding) + 1);
        if (encoding)
        strcpy(encoding, tok->encoding);
    }
    Ta3Tokenizer_Free(tok);
    return encoding;
}