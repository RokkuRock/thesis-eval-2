ast_for_call(struct compiling *c, const node *n, expr_ty func)
{
    int i, nargs, nkeywords, ngens;
    asdl_seq *args;
    asdl_seq *keywords;
    expr_ty vararg = NULL, kwarg = NULL;
    REQ(n, arglist);
    nargs = 0;
    nkeywords = 0;
    ngens = 0;
    for (i = 0; i < NCH(n); i++) {
        node *ch = CHILD(n, i);
        if (TYPE(ch) == argument) {
            if (NCH(ch) == 1)
                nargs++;
            else if (TYPE(CHILD(ch, 1)) == comp_for)
                ngens++;
            else
                nkeywords++;
        }
    }
    if (ngens > 1 || (ngens && (nargs || nkeywords))) {
        ast_error(n, "Generator expression must be parenthesized "
                  "if not sole argument");
        return NULL;
    }
    if (nargs + nkeywords + ngens > 255) {
      ast_error(n, "more than 255 arguments");
      return NULL;
    }
    args = asdl_seq_new(nargs + ngens, c->c_arena);
    if (!args)
        return NULL;
    keywords = asdl_seq_new(nkeywords, c->c_arena);
    if (!keywords)
        return NULL;
    nargs = 0;
    nkeywords = 0;
    for (i = 0; i < NCH(n); i++) {
        node *ch = CHILD(n, i);
        if (TYPE(ch) == argument) {
            expr_ty e;
            if (NCH(ch) == 1) {
                if (nkeywords) {
                    ast_error(CHILD(ch, 0),
                              "non-keyword arg after keyword arg");
                    return NULL;
                }
                if (vararg) {
                    ast_error(CHILD(ch, 0),
                              "only named arguments may follow *expression");
                    return NULL;
                }
                e = ast_for_expr(c, CHILD(ch, 0));
                if (!e)
                    return NULL;
                asdl_seq_SET(args, nargs++, e);
            }
            else if (TYPE(CHILD(ch, 1)) == comp_for) {
                e = ast_for_genexp(c, ch);
                if (!e)
                    return NULL;
                asdl_seq_SET(args, nargs++, e);
            }
            else {
                keyword_ty kw;
                identifier key;
                int k;
                char *tmp;
                e = ast_for_expr(c, CHILD(ch, 0));
                if (!e)
                    return NULL;
                if (e->kind == Lambda_kind) {
                    ast_error(CHILD(ch, 0),
                              "lambda cannot contain assignment");
                    return NULL;
                } else if (e->kind != Name_kind) {
                    ast_error(CHILD(ch, 0), "keyword can't be an expression");
                    return NULL;
                }
                key = e->v.Name.id;
                if (!forbidden_check(c, CHILD(ch, 0), PyUnicode_AsUTF8(key)))
                    return NULL;
                for (k = 0; k < nkeywords; k++) {
                    tmp = _PyUnicode_AsString(
                        ((keyword_ty)asdl_seq_GET(keywords, k))->arg);
                    if (!strcmp(tmp, _PyUnicode_AsString(key))) {
                        ast_error(CHILD(ch, 0), "keyword argument repeated");
                        return NULL;
                    }
                }
                e = ast_for_expr(c, CHILD(ch, 2));
                if (!e)
                    return NULL;
                kw = keyword(key, e, c->c_arena);
                if (!kw)
                    return NULL;
                asdl_seq_SET(keywords, nkeywords++, kw);
            }
        }
        else if (TYPE(ch) == STAR) {
            vararg = ast_for_expr(c, CHILD(n, i+1));
            if (!vararg)
                return NULL;
            i++;
        }
        else if (TYPE(ch) == DOUBLESTAR) {
            kwarg = ast_for_expr(c, CHILD(n, i+1));
            if (!kwarg)
                return NULL;
            i++;
        }
    }
    return Call(func, args, keywords, vararg, kwarg, func->lineno,
                func->col_offset, c->c_arena);
}