static Jsi_RC SysGetEnvCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    extern char **environ;
    char *cp;
    int i;
    if (interp->isSafe)
        return Jsi_LogError("no getenv in safe mode");
    Jsi_Value *v = Jsi_ValueArrayIndex(interp, args, 0);
    if (v != NULL) {
        const char *fnam = Jsi_ValueString(interp, v, NULL);
        if (!fnam) 
            return Jsi_LogError("arg1: expected string 'name'");
        cp = getenv(fnam);
        if (cp != NULL) {
            Jsi_ValueMakeStringDup(interp, ret, cp);
        }
        return JSI_OK;
    }
    Jsi_Value *vres;
    Jsi_Obj  *ores = Jsi_ObjNew(interp);
    Jsi_Value *nnv;
    char *val, nam[200];
    vres = Jsi_ValueMakeObject(interp, NULL, ores);
    for (i=0; ; i++) {
        int n;
        cp = environ[i];
        if (cp == 0 || ((val = Jsi_Strchr(cp, '='))==NULL))
            break;
        n = val-cp+1;
        if (n>=(int)sizeof(nam))
            n = sizeof(nam)-1;
        Jsi_Strncpy(nam, cp, n);
        val = val+1;
        nnv = Jsi_ValueMakeStringDup(interp, NULL, val);
        Jsi_ObjInsert(interp, ores, nam, nnv, 0);
    }
    Jsi_ValueReplace(interp, ret, vres);
    return JSI_OK;
}