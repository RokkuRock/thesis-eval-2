FstringParser_Finish(FstringParser *state, struct compiling *c,
                     const node *n)
{
    asdl_seq *seq;
    FstringParser_check_invariants(state);
    if(state->expr_list.size == 0) {
        if (!state->last_str) {
            state->last_str = PyUnicode_FromStringAndSize(NULL, 0);
            if (!state->last_str)
                goto error;
        }
        return make_str_node_and_del(&state->last_str, c, n);
    }
    if (state->last_str) {
        expr_ty str = make_str_node_and_del(&state->last_str, c, n);
        if (!str || ExprList_Append(&state->expr_list, str) < 0)
            goto error;
    }
    assert(state->last_str == NULL);
    seq = ExprList_Finish(&state->expr_list, c->c_arena);
    if (!seq)
        goto error;
    if (seq->size == 1)
        return seq->elements[0];
    return JoinedStr(seq, LINENO(n), n->n_col_offset, c->c_arena);
error:
    FstringParser_Dealloc(state);
    return NULL;
}