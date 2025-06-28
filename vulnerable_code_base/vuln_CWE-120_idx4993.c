static Jsi_RC DebugRemoveCmd_(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr, int op)
{
    Jsi_Value *val = Jsi_ValueArrayIndex(interp, args, 0);
    if (interp->breakpointHash)
    {
        int num;
        char nbuf[100];
        if (Jsi_GetIntFromValue(interp, val, &num) != JSI_OK) 
            return Jsi_LogError("bad number");
        snprintf(nbuf, sizeof(nbuf), "%d", num);
        Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->breakpointHash, nbuf);
        jsi_BreakPoint* bptr;
        if (hPtr && (bptr = (jsi_BreakPoint*)Jsi_HashValueGet(hPtr))) {
            switch (op) {
                case 1: bptr->enabled = 0; break;
                case 2: bptr->enabled = 1; break;
                default:
                    Jsi_HashEntryDelete(hPtr);
            }
            return JSI_OK;
        }
    }
    return Jsi_LogError("unknown breakpoint");
}