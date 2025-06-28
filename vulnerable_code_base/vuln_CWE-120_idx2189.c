unknown_layer_handler(TSS2_RC rc)
{
    static __thread char buf[32];
    clearbuf(buf);
    catbuf(buf, "0x%X", tpm2_error_get(rc));
    return buf;
}