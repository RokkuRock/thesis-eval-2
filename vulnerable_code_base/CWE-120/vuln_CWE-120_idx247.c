Tss2_RC_Decode(TSS2_RC rc)
{
    static __thread char buf[TSS2_ERR_LAYER_NAME_MAX + TSS2_ERR_LAYER_ERROR_STR_MAX + 1];
    clearbuf(buf);
    UINT8 layer = tss2_rc_layer_number_get(rc);
    TSS2_RC_HANDLER handler = layer_handler[layer].handler;
    const char *lname = layer_handler[layer].name;
    if (lname[0]) {
        catbuf(buf, "%s:", lname);
    } else {
        catbuf(buf, "%u:", layer);
    }
    handler = !handler ? unknown_layer_handler : handler;
    UINT16 err_bits = tpm2_error_get(rc);
    const char *e = err_bits ? handler(err_bits) : "success";
    if (e) {
        catbuf(buf, "%s", e);
    } else {
        catbuf(buf, "0x%X", err_bits);
    }
    return buf;
}