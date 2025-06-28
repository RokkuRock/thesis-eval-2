ast_for_comprehension(struct compiling *c, const node *n)
{
    int i, n_fors;
    asdl_seq *comps;
    n_fors = count_comp_fors(c, n);
    if (n_fors == -1)
        return NULL;
    comps = _Ta3_asdl_seq_new(n_fors, c->c_arena);
    if (!comps)
        return NULL;
    for (i = 0; i < n_fors; i++) {
        comprehension_ty comp;
        asdl_seq *t;
        expr_ty expression, first;
        node *for_ch;
        int is_async = 0;
        REQ(n, comp_for);
        if (TYPE(CHILD(n, 0)) == ASYNC) {
            is_async = 1;
        }
        if (is_async && c->c_feature_version < 6) {
            ast_error(c, n,
                    "Async comprehensions are only supported in Python 3.6 and greater");
            return NULL;
        }
        for_ch = CHILD(n, 1 + is_async);
        t = ast_for_exprlist(c, for_ch, Store);
        if (!t)
            return NULL;
        expression = ast_for_expr(c, CHILD(n, 3 + is_async));
        if (!expression)
            return NULL;
        first = (expr_ty)asdl_seq_GET(t, 0);
        if (NCH(for_ch) == 1)
            comp = comprehension(first, expression, NULL,
                                 is_async, c->c_arena);
        else
            comp = comprehension(Tuple(t, Store, first->lineno,
                                       first->col_offset, c->c_arena),
                                 expression, NULL, is_async, c->c_arena);
        if (!comp)
            return NULL;
        if (NCH(n) == (5 + is_async)) {
            int j, n_ifs;
            asdl_seq *ifs;
            n = CHILD(n, 4 + is_async);
            n_ifs = count_comp_ifs(c, n);
            if (n_ifs == -1)
                return NULL;
            ifs = _Ta3_asdl_seq_new(n_ifs, c->c_arena);
            if (!ifs)
                return NULL;
            for (j = 0; j < n_ifs; j++) {
                REQ(n, comp_iter);
                n = CHILD(n, 0);
                REQ(n, comp_if);
                expression = ast_for_expr(c, CHILD(n, 1));
                if (!expression)
                    return NULL;
                asdl_seq_SET(ifs, j, expression);
                if (NCH(n) == 3)
                    n = CHILD(n, 2);
            }
            if (TYPE(n) == comp_iter)
                n = CHILD(n, 0);
            comp->ifs = ifs;
        }
        asdl_seq_SET(comps, i, comp);
    }
    return comps;
}