ast_for_call(struct compiling *c, const node *n, expr_ty func)
{
    int i, nargs, nkeywords, ngens;
    int ndoublestars;
    asdl_seq *args;
    asdl_seq *keywords;
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
            else if (TYPE(CHILD(ch, 0)) == STAR)
                nargs++;
            else
                nkeywords++;
        }
    }
    if (ngens > 1 || (ngens && (nargs || nkeywords))) {
        ast_error(c, n, "Generator expression must be parenthesized "
                  "if not sole argument");
        return NULL;
    }
    if (nargs + nkeywords + ngens > 255) {
        ast_error(c, n, "more than 255 arguments");
        return NULL;
    }
    args = _Ta3_asdl_seq_new(nargs + ngens, c->c_arena);
    if (!args)
        return NULL;
    keywords = _Ta3_asdl_seq_new(nkeywords, c->c_arena);
    if (!keywords)
        return NULL;
    nargs = 0;   
    nkeywords = 0;   
    ndoublestars = 0;   
    for (i = 0; i < NCH(n); i++) {
        node *ch = CHILD(n, i);
        if (TYPE(ch) == argument) {
            expr_ty e;
            node *chch = CHILD(ch, 0);
            if (NCH(ch) == 1) {
                if (nkeywords) {
                    if (ndoublestars) {
                        ast_error(c, chch,
                                "positional argument follows "
                                "keyword argument unpacking");
                    }
                    else {
                        ast_error(c, chch,
                                "positional argument follows "
                                "keyword argument");
                    }
                    return NULL;
                }
                e = ast_for_expr(c, chch);
                if (!e)
                    return NULL;
                asdl_seq_SET(args, nargs++, e);
            }
            else if (TYPE(chch) == STAR) {
                expr_ty starred;
                if (ndoublestars) {
                    ast_error(c, chch,
                            "iterable argument unpacking follows "
                            "keyword argument unpacking");
                    return NULL;
                }
                e = ast_for_expr(c, CHILD(ch, 1));
                if (!e)
                    return NULL;
                starred = Starred(e, Load, LINENO(chch),
                        chch->n_col_offset,
                        c->c_arena);
                if (!starred)
                    return NULL;
                asdl_seq_SET(args, nargs++, starred);
            }
            else if (TYPE(chch) == DOUBLESTAR) {
                keyword_ty kw;
                i++;
                e = ast_for_expr(c, CHILD(ch, 1));
                if (!e)
                    return NULL;
                kw = keyword(NULL, e, c->c_arena);
                asdl_seq_SET(keywords, nkeywords++, kw);
                ndoublestars++;
            }
            else if (TYPE(CHILD(ch, 1)) == comp_for) {
                e = ast_for_genexp(c, ch);
                if (!e)
                    return NULL;
                asdl_seq_SET(args, nargs++, e);
            }
            else {
                keyword_ty kw;
                identifier key, tmp;
                int k;
                e = ast_for_expr(c, chch);
                if (!e)
                    return NULL;
                if (e->kind == Lambda_kind) {
                    ast_error(c, chch,
                            "lambda cannot contain assignment");
                    return NULL;
                }
                else if (e->kind != Name_kind) {
                    ast_error(c, chch,
                            "keyword can't be an expression");
                    return NULL;
                }
                else if (forbidden_name(c, e->v.Name.id, ch, 1)) {
                    return NULL;
                }
                key = e->v.Name.id;
                for (k = 0; k < nkeywords; k++) {
                    tmp = ((keyword_ty)asdl_seq_GET(keywords, k))->arg;
                    if (tmp && !PyUnicode_Compare(tmp, key)) {
                        ast_error(c, chch,
                                "keyword argument repeated");
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
    }
    return Call(func, args, keywords, func->lineno, func->col_offset, c->c_arena);
}