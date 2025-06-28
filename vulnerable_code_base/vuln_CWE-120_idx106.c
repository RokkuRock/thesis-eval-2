static Jsi_RC SysTimesCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    Jsi_RC rc = JSI_OK;
    int i, n=1, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Value *func = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_ValueIsBoolean(interp, func)) {
        bool bv;
        if (argc != 1)
            return Jsi_LogError("bool must be only arg");
        Jsi_GetBoolFromValue(interp, func, &bv);
        double now = jsi_GetTimestamp();
        if (bv)
            interp->timesStart = now;
        else {
            char buf[100];
            snprintf(buf, sizeof(buf), " (times = %.6f sec)\n", (now-interp->timesStart));
            Jsi_Puts(interp, jsi_Stderr, buf, -1);
        }
        Jsi_ValueMakeNumber(interp, ret, 0);
        return JSI_OK;
    }
    Jsi_Wide diff, start, end;
    if (!Jsi_ValueIsFunction(interp, func))
        return Jsi_LogError("arg1: expected function|bool");
    if (argc > 1 && Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, 1), &n) != JSI_OK)
        return JSI_ERROR;
    if (n<=0) 
        return Jsi_LogError("count not > 0: %d", n);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start = (Jsi_Wide) tv.tv_sec * 1000000 + tv.tv_usec;
    for (i=0; i<n && rc == JSI_OK; i++) {
        rc = Jsi_FunctionInvoke(interp, func, NULL, ret, NULL);
    }
    gettimeofday(&tv, NULL);
    end = (Jsi_Wide) tv.tv_sec * 1000000 + tv.tv_usec;
    diff = (end - start);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)diff);
    return rc;
}