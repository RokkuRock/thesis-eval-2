tok_get(struct tok_state *tok, char **p_start, char **p_end)
{
    int c;
    int blankline, nonascii;
    *p_start = *p_end = NULL;
  nextline:
    tok->start = NULL;
    blankline = 0;
    if (tok->atbol) {
        int col = 0;
        int altcol = 0;
        tok->atbol = 0;
        for (;;) {
            c = tok_nextc(tok);
            if (c == ' ') {
                col++, altcol++;
            }
            else if (c == '\t') {
                col = (col/tok->tabsize + 1) * tok->tabsize;
                altcol = (altcol/tok->alttabsize + 1)
                    * tok->alttabsize;
            }
            else if (c == '\014')  { 
                col = altcol = 0;  
            }
            else {
                break;
            }
        }
        tok_backup(tok, c);
        if (c == '#' || c == '\n') {
            if (col == 0 && c == '\n' && tok->prompt != NULL) {
                blankline = 0;  
            }
            else {
                blankline = 1;  
            }
        }
        if (!blankline && tok->level == 0) {
            if (col == tok->indstack[tok->indent]) {
                if (altcol != tok->altindstack[tok->indent]) {
                    if (indenterror(tok)) {
                        return ERRORTOKEN;
                    }
                }
            }
            else if (col > tok->indstack[tok->indent]) {
                if (tok->indent+1 >= MAXINDENT) {
                    tok->done = E_TOODEEP;
                    tok->cur = tok->inp;
                    return ERRORTOKEN;
                }
                if (altcol <= tok->altindstack[tok->indent]) {
                    if (indenterror(tok)) {
                        return ERRORTOKEN;
                    }
                }
                tok->pendin++;
                tok->indstack[++tok->indent] = col;
                tok->altindstack[tok->indent] = altcol;
            }
            else   {
                while (tok->indent > 0 &&
                    col < tok->indstack[tok->indent]) {
                    tok->pendin--;
                    tok->indent--;
                }
                if (col != tok->indstack[tok->indent]) {
                    tok->done = E_DEDENT;
                    tok->cur = tok->inp;
                    return ERRORTOKEN;
                }
                if (altcol != tok->altindstack[tok->indent]) {
                    if (indenterror(tok)) {
                        return ERRORTOKEN;
                    }
                }
            }
        }
    }
    tok->start = tok->cur;
    if (tok->pendin != 0) {
        if (tok->pendin < 0) {
            tok->pendin++;
            return DEDENT;
        }
        else {
            tok->pendin--;
            return INDENT;
        }
    }
    if (tok->async_def
        && !blankline
        && tok->level == 0
        && tok->async_def_nl
        && tok->async_def_indent >= tok->indent)
    {
        tok->async_def = 0;
        tok->async_def_indent = 0;
        tok->async_def_nl = 0;
    }
 again:
    tok->start = NULL;
    do {
        c = tok_nextc(tok);
    } while (c == ' ' || c == '\t' || c == '\014');
    tok->start = tok->cur - 1;
    if (c == '#') {
        const char *prefix, *p, *type_start;
        while (c != EOF && c != '\n')
            c = tok_nextc(tok);
        p = tok->start;
        prefix = type_comment_prefix;
        while (*prefix && p < tok->cur) {
            if (*prefix == ' ') {
                while (*p == ' ' || *p == '\t')
                    p++;
            } else if (*prefix == *p) {
                p++;
            } else {
                break;
            }
            prefix++;
        }
        if (!*prefix) {
            int is_type_ignore = 1;
            tok_backup(tok, c);   
            type_start = p;
            is_type_ignore = tok->cur >= p + 6 && memcmp(p, "ignore", 6) == 0;
            p += 6;
            while (is_type_ignore && p < tok->cur) {
              if (*p == '#')
                  break;
              is_type_ignore = is_type_ignore && (*p == ' ' || *p == '\t');
              p++;
            }
            if (is_type_ignore) {
                if (blankline) {
                    tok_nextc(tok);
                    tok->atbol = 1;
                }
                return TYPE_IGNORE;
            } else {
                *p_start = (char *) type_start;   
                *p_end = tok->cur;
                return TYPE_COMMENT;
            }
        }
    }
    if (c == EOF) {
        return tok->done == E_EOF ? ENDMARKER : ERRORTOKEN;
    }
    nonascii = 0;
    if (is_potential_identifier_start(c)) {
        int saw_b = 0, saw_r = 0, saw_u = 0, saw_f = 0;
        while (1) {
            if (!(saw_b || saw_u || saw_f) && (c == 'b' || c == 'B'))
                saw_b = 1;
            else if (!(saw_b || saw_u || saw_r || saw_f)
                     && (c == 'u'|| c == 'U')) {
                saw_u = 1;
            }
            else if (!(saw_r || saw_u) && (c == 'r' || c == 'R')) {
                saw_r = 1;
            }
            else if (!(saw_f || saw_b || saw_u) && (c == 'f' || c == 'F')) {
                saw_f = 1;
            }
            else {
                break;
            }
            c = tok_nextc(tok);
            if (c == '"' || c == '\'') {
                goto letter_quote;
            }
        }
        while (is_potential_identifier_char(c)) {
            if (c >= 128) {
                nonascii = 1;
            }
            c = tok_nextc(tok);
        }
        tok_backup(tok, c);
        if (nonascii && !verify_identifier(tok)) {
            return ERRORTOKEN;
        }
        *p_start = tok->start;
        *p_end = tok->cur;
        if (tok->cur - tok->start == 5) {
            if (tok->async_def) {
                if (memcmp(tok->start, "async", 5) == 0) {
                    return ASYNC;
                }
                if (memcmp(tok->start, "await", 5) == 0) {
                    return AWAIT;
                }
            }
            else if (memcmp(tok->start, "async", 5) == 0) {
                struct tok_state ahead_tok;
                char *ahead_tok_start = NULL, *ahead_tok_end = NULL;
                int ahead_tok_kind;
                memcpy(&ahead_tok, tok, sizeof(ahead_tok));
                ahead_tok_kind = tok_get(&ahead_tok, &ahead_tok_start,
                                         &ahead_tok_end);
                if (ahead_tok_kind == NAME
                    && ahead_tok.cur - ahead_tok.start == 3
                    && memcmp(ahead_tok.start, "def", 3) == 0)
                {
                    tok->async_def_indent = tok->indent;
                    tok->async_def = 1;
                    return ASYNC;
                }
            }
        }
        return NAME;
    }
    if (c == '\n') {
        tok->atbol = 1;
        if (blankline || tok->level > 0) {
            goto nextline;
        }
        *p_start = tok->start;
        *p_end = tok->cur - 1;  
        tok->cont_line = 0;
        if (tok->async_def) {
            tok->async_def_nl = 1;
        }
        return NEWLINE;
    }
    if (c == '.') {
        c = tok_nextc(tok);
        if (isdigit(c)) {
            goto fraction;
        } else if (c == '.') {
            c = tok_nextc(tok);
            if (c == '.') {
                *p_start = tok->start;
                *p_end = tok->cur;
                return ELLIPSIS;
            }
            else {
                tok_backup(tok, c);
            }
            tok_backup(tok, '.');
        }
        else {
            tok_backup(tok, c);
        }
        *p_start = tok->start;
        *p_end = tok->cur;
        return DOT;
    }
    if (isdigit(c)) {
        if (c == '0') {
            c = tok_nextc(tok);
            if (c == 'x' || c == 'X') {
                c = tok_nextc(tok);
                do {
                    if (c == '_') {
                        c = tok_nextc(tok);
                    }
                    if (!isxdigit(c)) {
                        tok->done = E_TOKEN;
                        tok_backup(tok, c);
                        return ERRORTOKEN;
                    }
                    do {
                        c = tok_nextc(tok);
                    } while (isxdigit(c));
                } while (c == '_');
            }
            else if (c == 'o' || c == 'O') {
                c = tok_nextc(tok);
                do {
                    if (c == '_') {
                        c = tok_nextc(tok);
                    }
                    if (c < '0' || c >= '8') {
                        tok->done = E_TOKEN;
                        tok_backup(tok, c);
                        return ERRORTOKEN;
                    }
                    do {
                        c = tok_nextc(tok);
                    } while ('0' <= c && c < '8');
                } while (c == '_');
            }
            else if (c == 'b' || c == 'B') {
                c = tok_nextc(tok);
                do {
                    if (c == '_') {
                        c = tok_nextc(tok);
                    }
                    if (c != '0' && c != '1') {
                        tok->done = E_TOKEN;
                        tok_backup(tok, c);
                        return ERRORTOKEN;
                    }
                    do {
                        c = tok_nextc(tok);
                    } while (c == '0' || c == '1');
                } while (c == '_');
            }
            else {
                int nonzero = 0;
                while (1) {
                    if (c == '_') {
                        c = tok_nextc(tok);
                        if (!isdigit(c)) {
                            tok->done = E_TOKEN;
                            tok_backup(tok, c);
                            return ERRORTOKEN;
                        }
                    }
                    if (c != '0') {
                        break;
                    }
                    c = tok_nextc(tok);
                }
                if (isdigit(c)) {
                    nonzero = 1;
                    c = tok_decimal_tail(tok);
                    if (c == 0) {
                        return ERRORTOKEN;
                    }
                }
                if (c == '.') {
                    c = tok_nextc(tok);
                    goto fraction;
                }
                else if (c == 'e' || c == 'E') {
                    goto exponent;
                }
                else if (c == 'j' || c == 'J') {
                    goto imaginary;
                }
                else if (nonzero) {
                    tok->done = E_TOKEN;
                    tok_backup(tok, c);
                    return ERRORTOKEN;
                }
            }
        }
        else {
            c = tok_decimal_tail(tok);
            if (c == 0) {
                return ERRORTOKEN;
            }
            {
                if (c == '.') {
                    c = tok_nextc(tok);
        fraction:
                    if (isdigit(c)) {
                        c = tok_decimal_tail(tok);
                        if (c == 0) {
                            return ERRORTOKEN;
                        }
                    }
                }
                if (c == 'e' || c == 'E') {
                    int e;
                  exponent:
                    e = c;
                    c = tok_nextc(tok);
                    if (c == '+' || c == '-') {
                        c = tok_nextc(tok);
                        if (!isdigit(c)) {
                            tok->done = E_TOKEN;
                            tok_backup(tok, c);
                            return ERRORTOKEN;
                        }
                    } else if (!isdigit(c)) {
                        tok_backup(tok, c);
                        tok_backup(tok, e);
                        *p_start = tok->start;
                        *p_end = tok->cur;
                        return NUMBER;
                    }
                    c = tok_decimal_tail(tok);
                    if (c == 0) {
                        return ERRORTOKEN;
                    }
                }
                if (c == 'j' || c == 'J') {
        imaginary:
                    c = tok_nextc(tok);
                }
            }
        }
        tok_backup(tok, c);
        *p_start = tok->start;
        *p_end = tok->cur;
        return NUMBER;
    }
  letter_quote:
    if (c == '\'' || c == '"') {
        int quote = c;
        int quote_size = 1;              
        int end_quote_size = 0;
        c = tok_nextc(tok);
        if (c == quote) {
            c = tok_nextc(tok);
            if (c == quote) {
                quote_size = 3;
            }
            else {
                end_quote_size = 1;      
            }
        }
        if (c != quote) {
            tok_backup(tok, c);
        }
        while (end_quote_size != quote_size) {
            c = tok_nextc(tok);
            if (c == EOF) {
                if (quote_size == 3) {
                    tok->done = E_EOFS;
                }
                else {
                    tok->done = E_EOLS;
                }
                tok->cur = tok->inp;
                return ERRORTOKEN;
            }
            if (quote_size == 1 && c == '\n') {
                tok->done = E_EOLS;
                tok->cur = tok->inp;
                return ERRORTOKEN;
            }
            if (c == quote) {
                end_quote_size += 1;
            }
            else {
                end_quote_size = 0;
                if (c == '\\') {
                    tok_nextc(tok);   
                }
            }
        }
        *p_start = tok->start;
        *p_end = tok->cur;
        return STRING;
    }
    if (c == '\\') {
        c = tok_nextc(tok);
        if (c != '\n') {
            tok->done = E_LINECONT;
            tok->cur = tok->inp;
            return ERRORTOKEN;
        }
        tok->cont_line = 1;
        goto again;  
    }
    {
        int c2 = tok_nextc(tok);
        int token = Ta3Token_TwoChars(c, c2);
        if (token != OP) {
            int c3 = tok_nextc(tok);
            int token3 = Ta3Token_ThreeChars(c, c2, c3);
            if (token3 != OP) {
                token = token3;
            }
            else {
                tok_backup(tok, c3);
            }
            *p_start = tok->start;
            *p_end = tok->cur;
            return token;
        }
        tok_backup(tok, c2);
    }
    switch (c) {
    case '(':
    case '[':
    case '{':
        tok->level++;
        break;
    case ')':
    case ']':
    case '}':
        tok->level--;
        break;
    }
    *p_start = tok->start;
    *p_end = tok->cur;
    return Ta3Token_OneChar(c);
}