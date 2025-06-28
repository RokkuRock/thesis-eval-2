ast_for_atom_expr(struct compiling *c, const node *n)
{
    int i, nch, start = 0;
    expr_ty e, tmp;
    REQ(n, atom_expr);
    nch = NCH(n);
    if (TYPE(CHILD(n, 0)) == AWAIT) {
        if (c->c_feature_version < 5) {
            ast_error(c, n,
                    "Await expressions are only supported in Python 3.5 and greater");
            return NULL;
        }
        start = 1;
        assert(nch > 1);
    }
    e = ast_for_atom(c, CHILD(n, start));
    if (!e)
        return NULL;
    if (nch == 1)
        return e;
    if (start && nch == 2) {
        return Await(e, LINENO(n), n->n_col_offset, c->c_arena);
    }
    for (i = start + 1; i < nch; i++) {
        node *ch = CHILD(n, i);
        if (TYPE(ch) != trailer)
            break;
        tmp = ast_for_trailer(c, ch, e);
        if (!tmp)
            return NULL;
        tmp->lineno = e->lineno;
        tmp->col_offset = e->col_offset;
        e = tmp;
    }
    if (start) {
        return Await(e, LINENO(n), n->n_col_offset, c->c_arena);
    }
    else {
        return e;
    }
}