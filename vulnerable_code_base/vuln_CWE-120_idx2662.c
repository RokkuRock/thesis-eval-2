Jsi_RC Jsi_RegExpMatch(Jsi_Interp *interp, Jsi_Value *pattern, const char *v, int *rc, Jsi_DString *dStr)
{
    Jsi_Regex *re;
    int regexec_flags = 0;
    if (rc)
        *rc = 0;
    if (pattern == NULL || pattern->vt != JSI_VT_OBJECT || pattern->d.obj->ot != JSI_OT_REGEXP) 
        return Jsi_LogError("expected pattern");
    re = pattern->d.obj->d.robj;
    regex_t *reg = &re->reg;
    regmatch_t pos = {};
    if (dStr)
        Jsi_DSInit(dStr);
    int r  = regexec(reg, v, 1, &pos, regexec_flags);
    if (r >= REG_BADPAT) {
        char buf[100];
        regerror(r, reg, buf, sizeof(buf));
        return Jsi_LogError("error while matching pattern: %s", buf);
    }
    if (r != REG_NOMATCH) {
        if (rc) *rc = 1;
        if (dStr && pos.rm_so >= 0 && pos.rm_eo >= 0 &&  pos.rm_eo >= pos.rm_so)
            Jsi_DSAppendLen(dStr, v + pos.rm_so, pos.rm_eo - pos.rm_so);
    }
    return JSI_OK;
}