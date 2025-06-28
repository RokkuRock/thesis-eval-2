ast_for_with_stmt(struct compiling *c, const node *n, int is_async)
{
    int i, n_items, nch_minus_type, has_type_comment;
    asdl_seq *items, *body;
    string type_comment;
    if (is_async && c->c_feature_version < 5) {
        ast_error(c, n,
                "Async with statements are only supported in Python 3.5 and greater");
        return NULL;
    }
    REQ(n, with_stmt);
    has_type_comment = TYPE(CHILD(n, NCH(n) - 2)) == TYPE_COMMENT;
    nch_minus_type = NCH(n) - has_type_comment;
    n_items = (nch_minus_type - 2) / 2;
    items = _Ta3_asdl_seq_new(n_items, c->c_arena);
    if (!items)
        return NULL;
    for (i = 1; i < nch_minus_type - 2; i += 2) {
        withitem_ty item = ast_for_with_item(c, CHILD(n, i));
        if (!item)
            return NULL;
        asdl_seq_SET(items, (i - 1) / 2, item);
    }
    body = ast_for_suite(c, CHILD(n, NCH(n) - 1));
    if (!body)
        return NULL;
    if (has_type_comment)
        type_comment = NEW_TYPE_COMMENT(CHILD(n, NCH(n) - 2));
    else
        type_comment = NULL;
    if (is_async)
        return AsyncWith(items, body, type_comment, LINENO(n), n->n_col_offset, c->c_arena);
    else
        return With(items, body, type_comment, LINENO(n), n->n_col_offset, c->c_arena);
}