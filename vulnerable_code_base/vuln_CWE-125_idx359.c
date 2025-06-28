ast_for_atom(struct compiling *c, const node *n)
{
    node *ch = CHILD(n, 0);
    switch (TYPE(ch)) {
    case NAME: {
        PyObject *name;
        const char *s = STR(ch);
        size_t len = strlen(s);
        if (len >= 4 && len <= 5) {
            if (!strcmp(s, "None"))
                return NameConstant(Py_None, LINENO(n), n->n_col_offset, c->c_arena);
            if (!strcmp(s, "True"))
                return NameConstant(Py_True, LINENO(n), n->n_col_offset, c->c_arena);
            if (!strcmp(s, "False"))
                return NameConstant(Py_False, LINENO(n), n->n_col_offset, c->c_arena);
        }
        name = new_identifier(s, c);
        if (!name)
            return NULL;
        return Name(name, Load, LINENO(n), n->n_col_offset, c->c_arena);
    }
    case STRING: {
        expr_ty str = parsestrplus(c, n);
        if (!str) {
            const char *errtype = NULL;
            if (PyErr_ExceptionMatches(PyExc_UnicodeError))
                errtype = "unicode error";
            else if (PyErr_ExceptionMatches(PyExc_ValueError))
                errtype = "value error";
            if (errtype) {
                char buf[128];
                const char *s = NULL;
                PyObject *type, *value, *tback, *errstr;
                PyErr_Fetch(&type, &value, &tback);
                errstr = PyObject_Str(value);
                if (errstr)
                    s = PyUnicode_AsUTF8(errstr);
                if (s) {
                    PyOS_snprintf(buf, sizeof(buf), "(%s) %s", errtype, s);
                } else {
                    PyErr_Clear();
                    PyOS_snprintf(buf, sizeof(buf), "(%s) unknown error", errtype);
                }
                Py_XDECREF(errstr);
                ast_error(c, n, buf);
                Py_DECREF(type);
                Py_XDECREF(value);
                Py_XDECREF(tback);
            }
            return NULL;
        }
        return str;
    }
    case NUMBER: {
        PyObject *pynum;
        const char *s = STR(ch);
        if (c->c_feature_version < 6 && strchr(s, '_') != NULL) {
            ast_error(c, ch,
                    "Underscores in numeric literals are only supported in Python 3.6 and greater");
            return NULL;
        }
        pynum = parsenumber(c, s);
        if (!pynum)
            return NULL;
        if (PyArena_AddPyObject(c->c_arena, pynum) < 0) {
            Py_DECREF(pynum);
            return NULL;
        }
        return Num(pynum, LINENO(n), n->n_col_offset, c->c_arena);
    }
    case ELLIPSIS:  
        return Ellipsis(LINENO(n), n->n_col_offset, c->c_arena);
    case LPAR:  
        ch = CHILD(n, 1);
        if (TYPE(ch) == RPAR)
            return Tuple(NULL, Load, LINENO(n), n->n_col_offset, c->c_arena);
        if (TYPE(ch) == yield_expr)
            return ast_for_expr(c, ch);
        if ((NCH(ch) > 1) && (TYPE(CHILD(ch, 1)) == comp_for))
            return ast_for_genexp(c, ch);
        return ast_for_testlist(c, ch);
    case LSQB:  
        ch = CHILD(n, 1);
        if (TYPE(ch) == RSQB)
            return List(NULL, Load, LINENO(n), n->n_col_offset, c->c_arena);
        REQ(ch, testlist_comp);
        if (NCH(ch) == 1 || TYPE(CHILD(ch, 1)) == COMMA) {
            asdl_seq *elts = seq_for_testlist(c, ch);
            if (!elts)
                return NULL;
            return List(elts, Load, LINENO(n), n->n_col_offset, c->c_arena);
        }
        else
            return ast_for_listcomp(c, ch);
    case LBRACE: {
        expr_ty res;
        ch = CHILD(n, 1);
        if (TYPE(ch) == RBRACE) {
            return Dict(NULL, NULL, LINENO(n), n->n_col_offset, c->c_arena);
        }
        else {
            int is_dict = (TYPE(CHILD(ch, 0)) == DOUBLESTAR);
            if (NCH(ch) == 1 ||
                    (NCH(ch) > 1 &&
                     TYPE(CHILD(ch, 1)) == COMMA)) {
                res = ast_for_setdisplay(c, ch);
            }
            else if (NCH(ch) > 1 &&
                    TYPE(CHILD(ch, 1)) == comp_for) {
                res = ast_for_setcomp(c, ch);
            }
            else if (NCH(ch) > 3 - is_dict &&
                    TYPE(CHILD(ch, 3 - is_dict)) == comp_for) {
                if (is_dict) {
                    ast_error(c, n, "dict unpacking cannot be used in "
                            "dict comprehension");
                    return NULL;
                }
                res = ast_for_dictcomp(c, ch);
            }
            else {
                res = ast_for_dictdisplay(c, ch);
            }
            if (res) {
                res->lineno = LINENO(n);
                res->col_offset = n->n_col_offset;
            }
            return res;
        }
    }
    default:
        PyErr_Format(PyExc_SystemError, "unhandled atom %d", TYPE(ch));
        return NULL;
    }
}