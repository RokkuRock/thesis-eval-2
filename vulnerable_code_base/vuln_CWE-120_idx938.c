static Jsi_RC NumberToExponentialCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[100];
    int prec = 0, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    if (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    snprintf(buf, sizeof(buf), "%.*" JSI_NUMEFMT, prec, num);
#ifdef __WIN32
    char *e = strrchr(buf, 'e');
    if (e && (e[1]=='+' || e[1]=='-')) {
        e++;
        int eNum = atoi(e);
        if (e[0]=='-')
            eNum = -eNum;
        e++;
        snprintf(e, (e-buf), "%02d", eNum);
    }
#endif
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}