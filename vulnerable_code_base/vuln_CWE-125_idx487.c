parsetok(struct tok_state *tok, grammar *g, int start, perrdetail *err_ret,
         int *flags)
{
    parser_state *ps;
    node *n;
    int started = 0;
    growable_int_array type_ignores;
    if (!growable_int_array_init(&type_ignores, 10)) {
        err_ret->error = E_NOMEM;
        Ta3Tokenizer_Free(tok);
        return NULL;
    }
    if ((ps = Ta3Parser_New(g, start)) == NULL) {
        err_ret->error = E_NOMEM;
        Ta3Tokenizer_Free(tok);
        return NULL;
    }
#ifdef PY_PARSER_REQUIRES_FUTURE_KEYWORD
    if (*flags & PyPARSE_BARRY_AS_BDFL)
        ps->p_flags |= CO_FUTURE_BARRY_AS_BDFL;
#endif
    for (;;) {
        char *a, *b;
        int type;
        size_t len;
        char *str;
        int col_offset;
        type = Ta3Tokenizer_Get(tok, &a, &b);
        if (type == ERRORTOKEN) {
            err_ret->error = tok->done;
            break;
        }
        if (type == ENDMARKER && started) {
            type = NEWLINE;  
            started = 0;
            if (tok->indent &&
                !(*flags & PyPARSE_DONT_IMPLY_DEDENT))
            {
                tok->pendin = -tok->indent;
                tok->indent = 0;
            }
        }
        else
            started = 1;
        len = b - a;  
        str = (char *) PyObject_MALLOC(len + 1);
        if (str == NULL) {
            err_ret->error = E_NOMEM;
            break;
        }
        if (len > 0)
            strncpy(str, a, len);
        str[len] = '\0';
#ifdef PY_PARSER_REQUIRES_FUTURE_KEYWORD
        if (type == NOTEQUAL) {
            if (!(ps->p_flags & CO_FUTURE_BARRY_AS_BDFL) &&
                            strcmp(str, "!=")) {
                PyObject_FREE(str);
                err_ret->error = E_SYNTAX;
                break;
            }
            else if ((ps->p_flags & CO_FUTURE_BARRY_AS_BDFL) &&
                            strcmp(str, "<>")) {
                PyObject_FREE(str);
                err_ret->text = "with Barry as BDFL, use '<>' "
                                "instead of '!='";
                err_ret->error = E_SYNTAX;
                break;
            }
        }
#endif
        if (a >= tok->line_start)
            col_offset = Py_SAFE_DOWNCAST(a - tok->line_start,
                                          intptr_t, int);
        else
            col_offset = -1;
        if (type == TYPE_IGNORE) {
            if (!growable_int_array_add(&type_ignores, tok->lineno)) {
                err_ret->error = E_NOMEM;
                break;
            }
            continue;
        }
        if ((err_ret->error =
             Ta3Parser_AddToken(ps, (int)type, str,
                               tok->lineno, col_offset,
                               &(err_ret->expected))) != E_OK) {
            if (err_ret->error != E_DONE) {
                PyObject_FREE(str);
                err_ret->token = type;
            }
            break;
        }
    }
    if (err_ret->error == E_DONE) {
        n = ps->p_tree;
        ps->p_tree = NULL;
        if (n->n_type == file_input) {
            int num;
            node *ch;
            size_t i;
            num = NCH(n);
            ch = CHILD(n, num - 1);
            REQ(ch, ENDMARKER);
            for (i = 0; i < type_ignores.num_items; i++) {
                Ta3Node_AddChild(ch, TYPE_IGNORE, NULL, type_ignores.items[i], 0);
            }
        }
        growable_int_array_deallocate(&type_ignores);
#ifndef PGEN
        if (start == single_input) {
            char *cur = tok->cur;
            char c = *tok->cur;
            for (;;) {
                while (c == ' ' || c == '\t' || c == '\n' || c == '\014')
                    c = *++cur;
                if (!c)
                    break;
                if (c != '#') {
                    err_ret->error = E_BADSINGLE;
                    Ta3Node_Free(n);
                    n = NULL;
                    break;
                }
                while (c && c != '\n')
                    c = *++cur;
            }
        }
#endif
    }
    else
        n = NULL;
#ifdef PY_PARSER_REQUIRES_FUTURE_KEYWORD
    *flags = ps->p_flags;
#endif
    Ta3Parser_Delete(ps);
    if (n == NULL) {
        if (tok->done == E_EOF)
            err_ret->error = E_EOF;
        err_ret->lineno = tok->lineno;
        if (tok->buf != NULL) {
            size_t len;
            assert(tok->cur - tok->buf < INT_MAX);
            err_ret->offset = (int)(tok->cur - tok->buf);
            len = tok->inp - tok->buf;
            err_ret->text = (char *) PyObject_MALLOC(len + 1);
            if (err_ret->text != NULL) {
                if (len > 0)
                    strncpy(err_ret->text, tok->buf, len);
                err_ret->text[len] = '\0';
            }
        }
    } else if (tok->encoding != NULL) {
        node* r = Ta3Node_New(encoding_decl);
        if (r)
            r->n_str = PyObject_MALLOC(strlen(tok->encoding)+1);
        if (!r || !r->n_str) {
            err_ret->error = E_NOMEM;
            if (r)
                PyObject_FREE(r);
            n = NULL;
            goto done;
        }
        strcpy(r->n_str, tok->encoding);
        PyMem_FREE(tok->encoding);
        tok->encoding = NULL;
        r->n_nchildren = 1;
        r->n_child = n;
        n = r;
    }
done:
    Ta3Tokenizer_Free(tok);
    return n;
}