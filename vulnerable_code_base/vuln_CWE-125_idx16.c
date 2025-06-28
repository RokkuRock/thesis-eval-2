FstringParser_ConcatFstring(FstringParser *state, const char **str,
                            const char *end, int raw, int recurse_lvl,
                            struct compiling *c, const node *n)
{
    FstringParser_check_invariants(state);
    while (1) {
        PyObject *literal = NULL;
        expr_ty expression = NULL;
        int result = fstring_find_literal_and_expr(str, end, raw, recurse_lvl,
                                                   &literal, &expression,
                                                   c, n);
        if (result < 0)
            return -1;
        if (!literal) {
        } else if (!state->last_str) {
            state->last_str = literal;
            literal = NULL;
        } else {
            assert(PyUnicode_GET_LENGTH(literal) != 0);
            if (FstringParser_ConcatAndDel(state, literal) < 0)
                return -1;
            literal = NULL;
        }
        assert(!state->last_str ||
               PyUnicode_GET_LENGTH(state->last_str) != 0);
        assert(literal == NULL);
        if (result == 1)
            continue;
        if (!expression)
            break;
        if (!state->last_str) {
        } else {
            expr_ty str = make_str_node_and_del(&state->last_str, c, n);
            if (!str || ExprList_Append(&state->expr_list, str) < 0)
                return -1;
        }
        if (ExprList_Append(&state->expr_list, expression) < 0)
            return -1;
    }
    if (recurse_lvl == 0 && *str < end-1) {
        ast_error(c, n, "f-string: unexpected end of string");
        return -1;
    }
    if (recurse_lvl != 0 && **str != '}') {
        ast_error(c, n, "f-string: expecting '}'");
        return -1;
    }
    FstringParser_check_invariants(state);
    return 0;
}