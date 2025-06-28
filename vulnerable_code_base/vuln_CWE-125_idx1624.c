string_object_to_c_ast(const char *s, PyObject *filename, int start,
                             PyCompilerFlags *flags, int feature_version,
                             PyArena *arena)
{
    mod_ty mod;
    PyCompilerFlags localflags;
    perrdetail err;
    int iflags = PARSER_FLAGS(flags);
    node *n = Ta3Parser_ParseStringObject(s, filename,
                                         &_Ta3Parser_Grammar, start, &err,
                                         &iflags);
    if (flags == NULL) {
        localflags.cf_flags = 0;
        flags = &localflags;
    }
    if (n) {
        flags->cf_flags |= iflags & PyCF_MASK;
        mod = Ta3AST_FromNodeObject(n, flags, filename, feature_version, arena);
        Ta3Node_Free(n);
    }
    else {
        err_input(&err);
        mod = NULL;
    }
    err_free(&err);
    return mod;
}