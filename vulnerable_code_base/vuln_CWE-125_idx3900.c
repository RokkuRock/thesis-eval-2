forbidden_name(struct compiling *c, identifier name, const node *n,
               int full_checks)
{
    assert(PyUnicode_Check(name));
    if (PyUnicode_CompareWithASCIIString(name, "__debug__") == 0) {
        ast_error(c, n, "assignment to keyword");
        return 1;
    }
    if (full_checks) {
        const char * const *p;
        for (p = FORBIDDEN; *p; p++) {
            if (PyUnicode_CompareWithASCIIString(name, *p) == 0) {
                ast_error(c, n, "assignment to keyword");
                return 1;
            }
        }
    }
    return 0;
}