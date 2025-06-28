static Jsi_RC SysVerConvertCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    Jsi_Value *flag = Jsi_ValueArrayIndex(interp, args, 1);
    if (!val) goto bail;
    if (Jsi_ValueIsNumber(interp, val)) {
        char buf[200];
        Jsi_Number n;
        if (Jsi_GetNumberFromValue(interp, val, &n) != JSI_OK)
            goto bail;
        jsi_VersionNormalize(n, buf, sizeof(buf));
        int trunc = 0;
        if (flag && (Jsi_GetIntFromValue(interp, flag, &trunc) != JSI_OK
            || trunc<0 || trunc>2))
            return Jsi_LogError("arg2: bad trunc: expected int between 0 and 2");
        if (trunc) {
            int len = Jsi_Strlen(buf)-1;
            while (trunc>0 && len>1) {
                if (buf[len] == '0' && buf[len-1] == '.')
                    buf[len-1] = 0;
                len -= 2;
                trunc--;
            }
        }
        Jsi_ValueMakeStringDup(interp, ret, buf);
        return JSI_OK;
    }
    if (Jsi_ValueIsString(interp, val)) {
        Jsi_Number n;
        if (jsi_GetVerFromVal(interp, val, &n, 0) == JSI_OK) {
            Jsi_ValueMakeNumber(interp, ret, n);
            return JSI_OK;
        }
    }
bail:
    Jsi_ValueMakeNull(interp, ret);
    return JSI_OK;
}