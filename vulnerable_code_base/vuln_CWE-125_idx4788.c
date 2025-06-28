ast_for_classdef(struct compiling *c, const node *n, asdl_seq *decorator_seq)
{
    PyObject *classname;
    asdl_seq *s;
    expr_ty call;
    REQ(n, classdef);
    if (NCH(n) == 4) {  
        s = ast_for_suite(c, CHILD(n, 3));
        if (!s)
            return NULL;
        classname = NEW_IDENTIFIER(CHILD(n, 1));
        if (!classname)
            return NULL;
        if (forbidden_name(c, classname, CHILD(n, 3), 0))
            return NULL;
        return ClassDef(classname, NULL, NULL, s, decorator_seq, LINENO(n),
                        n->n_col_offset, c->c_arena);
    }
    if (TYPE(CHILD(n, 3)) == RPAR) {  
        s = ast_for_suite(c, CHILD(n,5));
        if (!s)
            return NULL;
        classname = NEW_IDENTIFIER(CHILD(n, 1));
        if (!classname)
            return NULL;
        if (forbidden_name(c, classname, CHILD(n, 3), 0))
            return NULL;
        return ClassDef(classname, NULL, NULL, s, decorator_seq, LINENO(n),
                        n->n_col_offset, c->c_arena);
    }
    {
        PyObject *dummy_name;
        expr_ty dummy;
        dummy_name = NEW_IDENTIFIER(CHILD(n, 1));
        if (!dummy_name)
            return NULL;
        dummy = Name(dummy_name, Load, LINENO(n), n->n_col_offset, c->c_arena);
        call = ast_for_call(c, CHILD(n, 3), dummy);
        if (!call)
            return NULL;
    }
    s = ast_for_suite(c, CHILD(n, 6));
    if (!s)
        return NULL;
    classname = NEW_IDENTIFIER(CHILD(n, 1));
    if (!classname)
        return NULL;
    if (forbidden_name(c, classname, CHILD(n, 1), 0))
        return NULL;
    return ClassDef(classname, call->v.Call.args, call->v.Call.keywords, s,
                    decorator_seq, LINENO(n), n->n_col_offset, c->c_arena);
}