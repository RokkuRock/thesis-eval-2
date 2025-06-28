ast_for_funcdef(struct compiling *c, const node *n, asdl_seq *decorator_seq)
{
    return ast_for_funcdef_impl(c, n, decorator_seq,
                                0  );
}