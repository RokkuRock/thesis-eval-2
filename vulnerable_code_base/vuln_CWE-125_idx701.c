ast_for_async_stmt(struct compiling *c, const node *n)
{
    REQ(n, async_stmt);
    REQ(CHILD(n, 0), ASYNC);
    switch (TYPE(CHILD(n, 1))) {
        case funcdef:
            return ast_for_funcdef_impl(c, CHILD(n, 1), NULL,
                                        1  );
        case with_stmt:
            return ast_for_with_stmt(c, CHILD(n, 1),
                                     1  );
        case for_stmt:
            return ast_for_for_stmt(c, CHILD(n, 1),
                                    1  );
        default:
            PyErr_Format(PyExc_SystemError,
                         "invalid async stament: %s",
                         STR(CHILD(n, 1)));
            return NULL;
    }
}