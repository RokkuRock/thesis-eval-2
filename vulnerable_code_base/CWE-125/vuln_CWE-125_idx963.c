fstring_find_expr(const char **str, const char *end, int raw, int recurse_lvl,
                  expr_ty *expression, struct compiling *c, const node *n)
{
    const char *expr_start;
    const char *expr_end;
    expr_ty simple_expression;
    expr_ty format_spec = NULL;  
    int conversion = -1;  
    char quote_char = 0;
    int string_type = 0;
    Py_ssize_t nested_depth = 0;
    char parenstack[MAXLEVEL];
    if (recurse_lvl >= 2) {
        ast_error(c, n, "f-string: expressions nested too deeply");
        return -1;
    }
    assert(**str == '{');
    *str += 1;
    expr_start = *str;
    for (; *str < end; (*str)++) {
        char ch;
        assert(nested_depth >= 0);
        assert(*str >= expr_start && *str < end);
        if (quote_char)
            assert(string_type == 1 || string_type == 3);
        else
            assert(string_type == 0);
        ch = **str;
        if (ch == '\\') {
            ast_error(c, n, "f-string expression part "
                            "cannot include a backslash");
            return -1;
        }
        if (quote_char) {
            if (ch == quote_char) {
                if (string_type == 3) {
                    if (*str+2 < end && *(*str+1) == ch && *(*str+2) == ch) {
                        *str += 2;
                        string_type = 0;
                        quote_char = 0;
                        continue;
                    }
                } else {
                    quote_char = 0;
                    string_type = 0;
                    continue;
                }
            }
        } else if (ch == '\'' || ch == '"') {
            if (*str+2 < end && *(*str+1) == ch && *(*str+2) == ch) {
                string_type = 3;
                *str += 2;
            } else {
                string_type = 1;
            }
            quote_char = ch;
        } else if (ch == '[' || ch == '{' || ch == '(') {
            if (nested_depth >= MAXLEVEL) {
                ast_error(c, n, "f-string: too many nested parenthesis");
                return -1;
            }
            parenstack[nested_depth] = ch;
            nested_depth++;
        } else if (ch == '#') {
            ast_error(c, n, "f-string expression part cannot include '#'");
            return -1;
        } else if (nested_depth == 0 &&
                   (ch == '!' || ch == ':' || ch == '}')) {
            if (ch == '!' && *str+1 < end && *(*str+1) == '=') {
                continue;
            }
            break;
        } else if (ch == ']' || ch == '}' || ch == ')') {
            if (!nested_depth) {
                ast_error(c, n, "f-string: unmatched '%c'", ch);
                return -1;
            }
            nested_depth--;
            int opening = parenstack[nested_depth];
            if (!((opening == '(' && ch == ')') ||
                  (opening == '[' && ch == ']') ||
                  (opening == '{' && ch == '}')))
            {
                ast_error(c, n,
                          "f-string: closing parenthesis '%c' "
                          "does not match opening parenthesis '%c'",
                          ch, opening);
                return -1;
            }
        } else {
        }
    }
    expr_end = *str;
    if (quote_char) {
        ast_error(c, n, "f-string: unterminated string");
        return -1;
    }
    if (nested_depth) {
        int opening = parenstack[nested_depth - 1];
        ast_error(c, n, "f-string: unmatched '%c'", opening);
        return -1;
    }
    if (*str >= end)
        goto unexpected_end_of_string;
    simple_expression = fstring_compile_expr(expr_start, expr_end, c, n);
    if (!simple_expression)
        return -1;
    if (**str == '!') {
        *str += 1;
        if (*str >= end)
            goto unexpected_end_of_string;
        conversion = **str;
        *str += 1;
        if (!(conversion == 's' || conversion == 'r'
              || conversion == 'a')) {
            ast_error(c, n, "f-string: invalid conversion character: "
                            "expected 's', 'r', or 'a'");
            return -1;
        }
    }
    if (*str >= end)
        goto unexpected_end_of_string;
    if (**str == ':') {
        *str += 1;
        if (*str >= end)
            goto unexpected_end_of_string;
        format_spec = fstring_parse(str, end, raw, recurse_lvl+1, c, n);
        if (!format_spec)
            return -1;
    }
    if (*str >= end || **str != '}')
        goto unexpected_end_of_string;
    assert(*str < end);
    assert(**str == '}');
    *str += 1;
    *expression = FormattedValue(simple_expression, conversion,
                                 format_spec, LINENO(n), n->n_col_offset,
                                 n->n_end_lineno, n->n_end_col_offset,
                                 c->c_arena);
    if (!*expression)
        return -1;
    return 0;
unexpected_end_of_string:
    ast_error(c, n, "f-string: expecting '}'");
    return -1;
}