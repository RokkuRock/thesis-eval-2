count_comp_fors(struct compiling *c, const node *n)
{
    int n_fors = 0;
    int is_async;
  count_comp_for:
    is_async = 0;
    n_fors++;
    REQ(n, comp_for);
    if (TYPE(CHILD(n, 0)) == ASYNC) {
        is_async = 1;
    }
    if (NCH(n) == (5 + is_async)) {
        n = CHILD(n, 4 + is_async);
    }
    else {
        return n_fors;
    }
  count_comp_iter:
    REQ(n, comp_iter);
    n = CHILD(n, 0);
    if (TYPE(n) == comp_for)
        goto count_comp_for;
    else if (TYPE(n) == comp_if) {
        if (NCH(n) == 3) {
            n = CHILD(n, 2);
            goto count_comp_iter;
        }
        else
            return n_fors;
    }
    PyErr_SetString(PyExc_SystemError,
                    "logic error in count_comp_fors");
    return -1;
}