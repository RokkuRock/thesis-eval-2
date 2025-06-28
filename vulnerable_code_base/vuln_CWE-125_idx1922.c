ast_for_funcdef_impl(struct compiling *c, const node *n,
                     asdl_seq *decorator_seq, int is_async)
{
    identifier name;
    arguments_ty args;
    asdl_seq *body;
    expr_ty returns = NULL;
    int name_i = 1;
    node *tc;
    string type_comment = NULL;
    if (is_async && c->c_feature_version < 5) {
        ast_error(c, n,
                "Async functions are only supported in Python 3.5 and greater");
        return NULL;
    }
    REQ(n, funcdef);
    name = NEW_IDENTIFIER(CHILD(n, name_i));
    if (!name)
        return NULL;
    if (forbidden_name(c, name, CHILD(n, name_i), 0))
        return NULL;
    args = ast_for_arguments(c, CHILD(n, name_i + 1));
    if (!args)
        return NULL;
    if (TYPE(CHILD(n, name_i+2)) == RARROW) {
        returns = ast_for_expr(c, CHILD(n, name_i + 3));
        if (!returns)
            return NULL;
        name_i += 2;
    }
    if (TYPE(CHILD(n, name_i + 3)) == TYPE_COMMENT) {
        type_comment = NEW_TYPE_COMMENT(CHILD(n, name_i + 3));
        name_i += 1;
    }
    body = ast_for_suite(c, CHILD(n, name_i + 3));
    if (!body)
        return NULL;
    if (!type_comment && NCH(CHILD(n, name_i + 3)) > 1) {
        tc = CHILD(CHILD(n, name_i + 3), 1);
        if (TYPE(tc) == TYPE_COMMENT)
            type_comment = NEW_TYPE_COMMENT(tc);
    }
    if (is_async)
        return AsyncFunctionDef(name, args, body, decorator_seq, returns,
                                type_comment, LINENO(n),
                                n->n_col_offset, c->c_arena);
    else
        return FunctionDef(name, args, body, decorator_seq, returns,
                           type_comment, LINENO(n),
                           n->n_col_offset, c->c_arena);
}