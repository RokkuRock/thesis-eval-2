static Jsi_RC DebugInfoCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    if (!interp->breakpointHash) {
        Jsi_ValueMakeArrayObject(interp, ret, NULL);
        return JSI_OK;
    }
    int argc = Jsi_ValueGetLength(interp, args);
    if (argc == 0)
        return Jsi_HashKeysDump(interp, interp->breakpointHash, ret, 0);
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    int num;
    char nbuf[100];
    if (Jsi_GetIntFromValue(interp, val, &num) != JSI_OK) 
        return Jsi_LogError("bad number");
    snprintf(nbuf, sizeof(nbuf), "%d", num);
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->breakpointHash, nbuf);
    if (!hPtr) 
        return Jsi_LogError("unknown breakpoint");
    jsi_BreakPoint* bp = (jsi_BreakPoint*)Jsi_HashValueGet(hPtr);
    if (!bp) return JSI_ERROR;
    Jsi_DString dStr = {};
    if (bp->func)
        Jsi_DSPrintf(&dStr, "{id:%d, type:\"func\", func:\"%s\", hits:%d, enabled:%s, temporary:%s}",
         bp->id, bp->func, bp->hits, bp->enabled?"true":"false", bp->temp?"true":"false");
    else
        Jsi_DSPrintf(&dStr, "{id:%d, type:\"line\", file:\"%s\", line:%d, hits:%d, enabled:%s}",
            bp->id, bp->file?bp->file:"", bp->line, bp->hits, bp->enabled?"true":"false");
    Jsi_RC rc = Jsi_JSONParse(interp, Jsi_DSValue(&dStr), ret, 0);
    Jsi_DSFree(&dStr);
    return rc;
}