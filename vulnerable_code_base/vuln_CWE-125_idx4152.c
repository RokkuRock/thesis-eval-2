fstring_compile_expr(const char *expr_start, const char *expr_end,
                     struct compiling *c, const node *n)
{
    int all_whitespace = 1;
    int kind;
    void *data;
    PyCompilerFlags cf;
    mod_ty mod;
    char *str;
    PyObject *o, *fstring_name;
    Py_ssize_t len;
    Py_ssize_t i;
    assert(expr_end >= expr_start);
    assert(*(expr_start-1) == '{');
    assert(*expr_end == '}' || *expr_end == '!' || *expr_end == ':');
    o = PyUnicode_DecodeUTF8(expr_start, expr_end-expr_start, NULL);
    if (o == NULL)
        return NULL;
    len = PyUnicode_GET_LENGTH(o);
    kind = PyUnicode_KIND(o);
    data = PyUnicode_DATA(o);
    for (i = 0; i < len; i++) {
        if (!Py_UNICODE_ISSPACE(PyUnicode_READ(kind, data, i))) {
            all_whitespace = 0;
            break;
        }
    }
    Py_DECREF(o);
    if (all_whitespace) {
        ast_error(c, n, "f-string: empty expression not allowed");
        return NULL;
    }
    len = expr_end - expr_start;
    str = PyMem_RawMalloc(len + 3);
    if (str == NULL)
        return NULL;
    str[0] = '(';
    memcpy(str+1, expr_start, len);
    str[len+1] = ')';
    str[len+2] = 0;
    cf.cf_flags = PyCF_ONLY_AST;
    fstring_name = PyUnicode_FromString("<fstring>");
    mod = string_object_to_c_ast(str, fstring_name,
                                 Py_eval_input, &cf,
                                 c->c_feature_version, c->c_arena);
    Py_DECREF(fstring_name);
    PyMem_RawFree(str);
    if (!mod)
        return NULL;
    return mod->v.Expression.body;
}