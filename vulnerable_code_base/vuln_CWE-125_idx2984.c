ast_for_arguments(struct compiling *c, const node *n)
{
    int i, j, k, nposargs = 0, nkwonlyargs = 0;
    int nposdefaults = 0, found_default = 0;
    asdl_seq *posargs, *posdefaults, *kwonlyargs, *kwdefaults;
    arg_ty vararg = NULL, kwarg = NULL;
    arg_ty arg = NULL;
    node *ch;
    if (TYPE(n) == parameters) {
        if (NCH(n) == 2)  
            return arguments(NULL, NULL, NULL, NULL, NULL, NULL, c->c_arena);
        n = CHILD(n, 1);
    }
    assert(TYPE(n) == typedargslist || TYPE(n) == varargslist);
    for (i = 0; i < NCH(n); i++) {
        ch = CHILD(n, i);
        if (TYPE(ch) == STAR) {
            i++;
            if (i < NCH(n) &&  
                (TYPE(CHILD(n, i)) == tfpdef ||
                 TYPE(CHILD(n, i)) == vfpdef)) {
                i++;
            }
            break;
        }
        if (TYPE(ch) == DOUBLESTAR) break;
        if (TYPE(ch) == vfpdef || TYPE(ch) == tfpdef) nposargs++;
        if (TYPE(ch) == EQUAL) nposdefaults++;
    }
    for ( ; i < NCH(n); ++i) {
        ch = CHILD(n, i);
        if (TYPE(ch) == DOUBLESTAR) break;
        if (TYPE(ch) == tfpdef || TYPE(ch) == vfpdef) nkwonlyargs++;
    }
    posargs = (nposargs ? _Py_asdl_seq_new(nposargs, c->c_arena) : NULL);
    if (!posargs && nposargs)
        return NULL;
    kwonlyargs = (nkwonlyargs ?
                   _Py_asdl_seq_new(nkwonlyargs, c->c_arena) : NULL);
    if (!kwonlyargs && nkwonlyargs)
        return NULL;
    posdefaults = (nposdefaults ?
                    _Py_asdl_seq_new(nposdefaults, c->c_arena) : NULL);
    if (!posdefaults && nposdefaults)
        return NULL;
    kwdefaults = (nkwonlyargs ?
                   _Py_asdl_seq_new(nkwonlyargs, c->c_arena) : NULL);
    if (!kwdefaults && nkwonlyargs)
        return NULL;
    i = 0;
    j = 0;   
    k = 0;   
    while (i < NCH(n)) {
        ch = CHILD(n, i);
        switch (TYPE(ch)) {
            case tfpdef:
            case vfpdef:
                if (i + 1 < NCH(n) && TYPE(CHILD(n, i + 1)) == EQUAL) {
                    expr_ty expression = ast_for_expr(c, CHILD(n, i + 2));
                    if (!expression)
                        return NULL;
                    assert(posdefaults != NULL);
                    asdl_seq_SET(posdefaults, j++, expression);
                    i += 2;
                    found_default = 1;
                }
                else if (found_default) {
                    ast_error(c, n,
                              "non-default argument follows default argument");
                    return NULL;
                }
                arg = ast_for_arg(c, ch);
                if (!arg)
                    return NULL;
                asdl_seq_SET(posargs, k++, arg);
                i += 1;  
                if (i < NCH(n) && TYPE(CHILD(n, i)) == COMMA)
                    i += 1;  
                break;
            case STAR:
                if (i+1 >= NCH(n) ||
                    (i+2 == NCH(n) && (TYPE(CHILD(n, i+1)) == COMMA
                                       || TYPE(CHILD(n, i+1)) == TYPE_COMMENT))) {
                    ast_error(c, CHILD(n, i),
                              "named arguments must follow bare *");
                    return NULL;
                }
                ch = CHILD(n, i+1);   
                if (TYPE(ch) == COMMA) {
                    int res = 0;
                    i += 2;  
                    if (i < NCH(n) && TYPE(CHILD(n, i)) == TYPE_COMMENT) {
                        ast_error(c, CHILD(n, i),
                                  "bare * has associated type comment");
                        return NULL;
                    }
                    res = handle_keywordonly_args(c, n, i,
                                                  kwonlyargs, kwdefaults);
                    if (res == -1) return NULL;
                    i = res;  
                }
                else {
                    vararg = ast_for_arg(c, ch);
                    if (!vararg)
                        return NULL;
                i += 2;  
                if (i < NCH(n) && TYPE(CHILD(n, i)) == COMMA)
                    i += 1;  
                if (i < NCH(n) && TYPE(CHILD(n, i)) == TYPE_COMMENT) {
                        vararg->type_comment = NEW_TYPE_COMMENT(CHILD(n, i));
                        if (!vararg->type_comment)
                            return NULL;
                        i += 1;
                    }
                    if (i < NCH(n) && (TYPE(CHILD(n, i)) == tfpdef
                                    || TYPE(CHILD(n, i)) == vfpdef)) {
                        int res = 0;
                        res = handle_keywordonly_args(c, n, i,
                                                      kwonlyargs, kwdefaults);
                        if (res == -1) return NULL;
                        i = res;  
                    }
                }
                break;
            case DOUBLESTAR:
                ch = CHILD(n, i+1);   
                assert(TYPE(ch) == tfpdef || TYPE(ch) == vfpdef);
                kwarg = ast_for_arg(c, ch);
                if (!kwarg)
                    return NULL;
                i += 2;  
                if (TYPE(CHILD(n, i)) == COMMA)
                    i += 1;  
                break;
            case TYPE_COMMENT:
                assert(i);
                if (kwarg)
                    arg = kwarg;
                arg->type_comment = NEW_TYPE_COMMENT(ch);
                if (!arg->type_comment)
                    return NULL;
                i += 1;
                break;
            default:
                PyErr_Format(PyExc_SystemError,
                             "unexpected node in varargslist: %d @ %d",
                             TYPE(ch), i);
                return NULL;
        }
    }
    return arguments(posargs, vararg, kwonlyargs, kwdefaults, kwarg, posdefaults, c->c_arena);
}