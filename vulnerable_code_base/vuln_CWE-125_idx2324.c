ast_for_async_funcdef(struct compiling *c, const node *n, asdl_seq *decorator_seq)
{
    REQ(n, async_funcdef);
    REQ(CHILD(n, 0), ASYNC);
    REQ(CHILD(n, 1), funcdef);
    return ast_for_funcdef_impl(c, CHILD(n, 1), decorator_seq,
                                1  );
}