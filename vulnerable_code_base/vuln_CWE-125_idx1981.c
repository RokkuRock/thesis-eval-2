ast_for_atom(struct compiling *c, const node *n)
{
    node *ch = CHILD(n, 0);
    switch (TYPE(ch)) {
    case NAME: {
        PyObject *name = NEW_IDENTIFIER(ch);
        if (!name)
            return NULL;
        return Name(name, Load, LINENO(n), n->n_col_offset, c->c_arena);
    }
    case STRING: {
        PyObject *kind, *str = parsestrplus(c, n);
        const char *raw, *s = STR(CHILD(n, 0));
        int quote = Py_CHARMASK(*s);
        char *ch, s_kind[3] = {0, 0, 0};
        ch = s_kind;
        raw = s;
        while (*raw && *raw != '\'' && *raw != '"') {
            *ch++ = *raw++;
        }
        kind = PyUnicode_FromString(s_kind);
        if (!kind) {
            return NULL;
        }
        if (!str) {
#ifdef Py_USING_UNICODE
            if (PyErr_ExceptionMatches(PyExc_UnicodeError)){
                PyObject *type, *value, *tback, *errstr;
                PyErr_Fetch(&type, &value, &tback);
                errstr = PyObject_Str(value);
                if (errstr) {
                    char *s = "";
                    char buf[128];
                    s = _PyUnicode_AsString(errstr);
                    PyOS_snprintf(buf, sizeof(buf), "(unicode error) %s", s);
                    ast_error(n, buf);
                    Py_DECREF(errstr);
                } else {
                    ast_error(n, "(unicode error) unknown error");
                }
                Py_DECREF(type);
                Py_DECREF(value);
                Py_XDECREF(tback);
            }
#endif
            return NULL;
        }
        PyArena_AddPyObject(c->c_arena, str);
        return Str(str, kind, LINENO(n), n->n_col_offset, c->c_arena);
    }
    case NUMBER: {
        PyObject *pynum = parsenumber(c, STR(ch));
        if (!pynum)
            return NULL;
        PyArena_AddPyObject(c->c_arena, pynum);
        return Num(pynum, LINENO(n), n->n_col_offset, c->c_arena);
    }
    case LPAR:  
        ch = CHILD(n, 1);
        if (TYPE(ch) == RPAR)
            return Tuple(NULL, Load, LINENO(n), n->n_col_offset, c->c_arena);
        if (TYPE(ch) == yield_expr)
            return ast_for_expr(c, ch);
        return ast_for_testlist_comp(c, ch);
    case LSQB:  
        ch = CHILD(n, 1);
        if (TYPE(ch) == RSQB)
            return List(NULL, Load, LINENO(n), n->n_col_offset, c->c_arena);
        REQ(ch, listmaker);
        if (NCH(ch) == 1 || TYPE(CHILD(ch, 1)) == COMMA) {
            asdl_seq *elts = seq_for_testlist(c, ch);
            if (!elts)
                return NULL;
            return List(elts, Load, LINENO(n), n->n_col_offset, c->c_arena);
        }
        else
            return ast_for_listcomp(c, ch);
    case LBRACE: {
        int i, size;
        asdl_seq *keys, *values;
        ch = CHILD(n, 1);
        if (TYPE(ch) == RBRACE) {
            return Dict(NULL, NULL, LINENO(n), n->n_col_offset, c->c_arena);
        } else if (NCH(ch) == 1 || TYPE(CHILD(ch, 1)) == COMMA) {
            asdl_seq *elts;
            size = (NCH(ch) + 1) / 2;  
            elts = asdl_seq_new(size, c->c_arena);
            if (!elts)
                return NULL;
            for (i = 0; i < NCH(ch); i += 2) {
                expr_ty expression;
                expression = ast_for_expr(c, CHILD(ch, i));
                if (!expression)
                    return NULL;
                asdl_seq_SET(elts, i / 2, expression);
            }
            return Set(elts, LINENO(n), n->n_col_offset, c->c_arena);
        } else if (TYPE(CHILD(ch, 1)) == comp_for) {
            return ast_for_setcomp(c, ch);
        } else if (NCH(ch) > 3 && TYPE(CHILD(ch, 3)) == comp_for) {
            return ast_for_dictcomp(c, ch);
        } else {
            size = (NCH(ch) + 1) / 4;  
            keys = asdl_seq_new(size, c->c_arena);
            if (!keys)
                return NULL;
            values = asdl_seq_new(size, c->c_arena);
            if (!values)
                return NULL;
            for (i = 0; i < NCH(ch); i += 4) {
                expr_ty expression;
                expression = ast_for_expr(c, CHILD(ch, i));
                if (!expression)
                    return NULL;
                asdl_seq_SET(keys, i / 4, expression);
                expression = ast_for_expr(c, CHILD(ch, i + 2));
                if (!expression)
                    return NULL;
                asdl_seq_SET(values, i / 4, expression);
            }
            return Dict(keys, values, LINENO(n), n->n_col_offset, c->c_arena);
        }
    }
    case BACKQUOTE: {  
        expr_ty expression;
        if (Py_Py3kWarningFlag &&
            !ast_warn(c, n, "backquote not supported in 3.x; use repr()"))
            return NULL;
        expression = ast_for_testlist(c, CHILD(n, 1));
        if (!expression)
            return NULL;
        return Repr(expression, LINENO(n), n->n_col_offset, c->c_arena);
    }
    default:
        PyErr_Format(PyExc_SystemError, "unhandled atom %d", TYPE(ch));
        return NULL;
    }
}