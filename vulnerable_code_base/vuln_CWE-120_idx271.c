static Jsi_RC WebSocketVersionCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    const char *verStr = NULL;
    verStr = lws_get_library_version();
    if (verStr) {
        char buf[100], *cp;
        snprintf(buf, sizeof(buf), "%s", verStr);
        cp = Jsi_Strchr(buf, ' ');
        if (cp) *cp = 0;
        Jsi_ValueMakeStringDup(interp, ret, buf);
    }
    return JSI_OK;
}