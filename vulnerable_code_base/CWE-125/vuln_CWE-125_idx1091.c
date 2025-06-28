Ta3AST_FromNodeObject(const node *n, PyCompilerFlags *flags,
                     PyObject *filename, int feature_version,
                     PyArena *arena)
{
    int i, j, k, num;
    asdl_seq *stmts = NULL;
    asdl_seq *type_ignores = NULL;
    stmt_ty s;
    node *ch;
    struct compiling c;
    mod_ty res = NULL;
    asdl_seq *argtypes = NULL;
    expr_ty ret, arg;
    c.c_arena = arena;
    c.c_filename = filename;
    c.c_normalize = NULL;
    c.c_normalize_args = NULL;
    c.c_feature_version = feature_version;
    if (TYPE(n) == encoding_decl)
        n = CHILD(n, 0);
    k = 0;
    switch (TYPE(n)) {
        case file_input:
            stmts = _Ta3_asdl_seq_new(num_stmts(n), arena);
            if (!stmts)
                goto out;
            for (i = 0; i < NCH(n) - 1; i++) {
                ch = CHILD(n, i);
                if (TYPE(ch) == NEWLINE)
                    continue;
                REQ(ch, stmt);
                num = num_stmts(ch);
                if (num == 1) {
                    s = ast_for_stmt(&c, ch);
                    if (!s)
                        goto out;
                    asdl_seq_SET(stmts, k++, s);
                }
                else {
                    ch = CHILD(ch, 0);
                    REQ(ch, simple_stmt);
                    for (j = 0; j < num; j++) {
                        s = ast_for_stmt(&c, CHILD(ch, j * 2));
                        if (!s)
                            goto out;
                        asdl_seq_SET(stmts, k++, s);
                    }
                }
            }
            ch = CHILD(n, NCH(n) - 1);
            REQ(ch, ENDMARKER);
            num = NCH(ch);
            type_ignores = _Ta3_asdl_seq_new(num, arena);
            if (!type_ignores)
                goto out;
            for (i = 0; i < num; i++) {
                type_ignore_ty ti = TypeIgnore(LINENO(CHILD(ch, i)), arena);
                if (!ti)
                    goto out;
                asdl_seq_SET(type_ignores, i, ti);
            }
            res = Module(stmts, type_ignores, arena);
            break;
        case eval_input: {
            expr_ty testlist_ast;
            testlist_ast = ast_for_testlist(&c, CHILD(n, 0));
            if (!testlist_ast)
                goto out;
            res = Expression(testlist_ast, arena);
            break;
        }
        case single_input:
            if (TYPE(CHILD(n, 0)) == NEWLINE) {
                stmts = _Ta3_asdl_seq_new(1, arena);
                if (!stmts)
                    goto out;
                asdl_seq_SET(stmts, 0, Pass(n->n_lineno, n->n_col_offset,
                                            arena));
                if (!asdl_seq_GET(stmts, 0))
                    goto out;
                res = Interactive(stmts, arena);
            }
            else {
                n = CHILD(n, 0);
                num = num_stmts(n);
                stmts = _Ta3_asdl_seq_new(num, arena);
                if (!stmts)
                    goto out;
                if (num == 1) {
                    s = ast_for_stmt(&c, n);
                    if (!s)
                        goto out;
                    asdl_seq_SET(stmts, 0, s);
                }
                else {
                    REQ(n, simple_stmt);
                    for (i = 0; i < NCH(n); i += 2) {
                        if (TYPE(CHILD(n, i)) == NEWLINE)
                            break;
                        s = ast_for_stmt(&c, CHILD(n, i));
                        if (!s)
                            goto out;
                        asdl_seq_SET(stmts, i / 2, s);
                    }
                }
                res = Interactive(stmts, arena);
            }
            break;
        case func_type_input:
            n = CHILD(n, 0);
            REQ(n, func_type);
            if (TYPE(CHILD(n, 1)) == typelist) {
                ch = CHILD(n, 1);
                num = 0;
                for (i = 0; i < NCH(ch); i++) {
                    if (TYPE(CHILD(ch, i)) == test)
                        num++;
                }
                argtypes = _Ta3_asdl_seq_new(num, arena);
                j = 0;
                for (i = 0; i < NCH(ch); i++) {
                    if (TYPE(CHILD(ch, i)) == test) {
                        arg = ast_for_expr(&c, CHILD(ch, i));
                        if (!arg)
                            goto out;
                        asdl_seq_SET(argtypes, j++, arg);
                    }
                }
            }
            else
                argtypes = _Ta3_asdl_seq_new(0, arena);
            ret = ast_for_expr(&c, CHILD(n, NCH(n) - 1));
            if (!ret)
                goto out;
            res = FunctionType(argtypes, ret, arena);
            break;
        default:
            PyErr_Format(PyExc_SystemError,
                         "invalid node %d for Ta3AST_FromNode", TYPE(n));
            goto out;
    }
 out:
    if (c.c_normalize) {
        Py_DECREF(c.c_normalize);
        PyTuple_SET_ITEM(c.c_normalize_args, 1, NULL);
        Py_DECREF(c.c_normalize_args);
    }
    return res;
}