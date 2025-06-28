test_custom_handler(void **state)
{
    (void) state;
    TSS2_RC_HANDLER old = Tss2_RC_SetHandler(1, "cstm", custom_err_handler);
    assert_null(old);
    unsigned i;
    for (i = 1; i < 4; i++) {
        TSS2_RC rc = TSS2_RC_LAYER(1) | i;
        char buf[256];
        snprintf(buf, sizeof(buf), "cstm:error %u", i);
        const char *e = Tss2_RC_Decode(rc);
        assert_string_equal(e, buf);
    }
    TSS2_RC rc = TSS2_RC_LAYER(1) | 42;
    const char *e = Tss2_RC_Decode(rc);
    assert_string_equal(e, "cstm:0x2A");
    old = Tss2_RC_SetHandler(1, "cstm", NULL);
    assert_ptr_equal(old, custom_err_handler);
    e = Tss2_RC_Decode(rc);
    assert_string_equal(e, "1:0x2A");
}