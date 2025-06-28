static Jsi_RC NumberToFixedCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[100];
    int prec = 0, skip = 0;
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    Jsi_Value *pa = Jsi_ValueArrayIndex(interp, args, skip);
    if (pa && Jsi_GetIntFromValue(interp, pa, &prec) != JSI_OK)
        return JSI_ERROR;
    if (prec<0) prec = 0;
    Jsi_GetDoubleFromValue(interp, v, &num);
    snprintf(buf, sizeof(buf), "%.*" JSI_NUMFFMT, prec, num);
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}