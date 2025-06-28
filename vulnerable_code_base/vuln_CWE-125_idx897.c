fstring_find_literal(const char **str, const char *end, int raw,
                     PyObject **literal, int recurse_lvl,
                     struct compiling *c, const node *n)
{
    const char *literal_start = *str;
    const char *literal_end;
    int in_named_escape = 0;
    int result = 0;
    assert(*literal == NULL);
    for (; *str < end; (*str)++) {
        char ch = **str;
        if (!in_named_escape && ch == '{' && (*str)-literal_start >= 2 &&
            *(*str-2) == '\\' && *(*str-1) == 'N') {
            in_named_escape = 1;
        } else if (in_named_escape && ch == '}') {
            in_named_escape = 0;
        } else if (ch == '{' || ch == '}') {
            if (recurse_lvl == 0) {
                if (*str+1 < end && *(*str+1) == ch) {
                    literal_end = *str+1;
                    *str += 2;
                    result = 1;
                    goto done;
                }
                if (ch == '}') {
                    ast_error(c, n, "f-string: single '}' is not allowed");
                    return -1;
                }
            }
            break;
        }
    }
    literal_end = *str;
    assert(*str <= end);
    assert(*str == end || **str == '{' || **str == '}');
done:
    if (literal_start != literal_end) {
        if (raw)
            *literal = PyUnicode_DecodeUTF8Stateful(literal_start,
                                                    literal_end-literal_start,
                                                    NULL, NULL);
        else
            *literal = decode_unicode_with_escapes(c, n, literal_start,
                                                   literal_end-literal_start);
        if (!*literal)
            return -1;
    }
    return result;
}