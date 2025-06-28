static Jsi_RC DebugAddCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (!interp->breakpointHash)
        interp->breakpointHash = Jsi_HashNew(interp, JSI_KEYS_STRING, jsi_HashFree);
    int argc = Jsi_ValueGetLength(interp, args);
    jsi_BreakPoint *bptr, bp = {};
    Jsi_Number vnum;
    if (argc>1 && Jsi_ValueGetBoolean(interp, Jsi_ValueArrayIndex(interp, args, 1), &bp.temp) != JSI_OK) 
        return Jsi_LogError("bad boolean");
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (Jsi_ValueGetNumber(interp, v, &vnum) == JSI_OK) {
        bp.line = (int)vnum;
        bp.file = interp->curFile;
    } else {
        const char *val = Jsi_ValueArrayIndexToStr(interp, args, 0, NULL);
        const char *cp;
        if (isdigit(val[0])) {
            if (Jsi_GetInt(interp, val, &bp.line, 0) != JSI_OK) 
                return Jsi_LogError("bad number");
            bp.file = interp->curFile;
        } else if ((cp = Jsi_Strchr(val, ':'))) {
            if (Jsi_GetInt(interp, cp+1, &bp.line, 0) != JSI_OK) 
                return Jsi_LogError("bad number");
            Jsi_DString dStr = {};
            Jsi_DSAppendLen(&dStr, val, cp-val);
            bp.file = Jsi_KeyAdd(interp, Jsi_DSValue(&dStr));
            Jsi_DSFree(&dStr);
        } else {
            bp.func = Jsi_KeyAdd(interp, val);
        }
    }
    if (bp.line<=0 && !bp.func) 
        return Jsi_LogError("bad number");
    char nbuf[100];
    bp.id = ++interp->debugOpts.breakIdx;
    bp.enabled = 1;
    snprintf(nbuf, sizeof(nbuf), "%d", bp.id);
    bptr = (jsi_BreakPoint*)Jsi_Malloc(sizeof(*bptr));
    *bptr = bp;
    Jsi_HashSet(interp->breakpointHash, (void*)nbuf, bptr);
    Jsi_ValueMakeNumber(interp, ret, (Jsi_Number)bp.id);
    return JSI_OK;
}