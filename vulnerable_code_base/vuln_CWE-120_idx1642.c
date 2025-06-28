Jsi_RC jsi_PkgDumpInfo(Jsi_Interp *interp, const char *name, Jsi_Value **ret) {
    jsi_PkgInfo *ptr;
    Jsi_HashEntry *hPtr = Jsi_HashEntryFind(interp->packageHash, name);
    if (hPtr && ((ptr = (jsi_PkgInfo*)Jsi_HashValueGet(hPtr)))) {
        Jsi_Obj *nobj = Jsi_ObjNew(interp);
        Jsi_ValueMakeObject(interp, ret, nobj);
        Jsi_ObjInsert(interp, nobj, "name", Jsi_ValueNewStringDup(interp, name), 0);
        Jsi_ObjInsert(interp, nobj, "version", Jsi_ValueNewNumber(interp, ptr->version), 0);
        Jsi_ObjInsert(interp, nobj, "lastReq", Jsi_ValueNewNumber(interp, ptr->lastReq), 0);
        char buf[200];
        jsi_VersionNormalize(ptr->version, buf, sizeof(buf));
        Jsi_ObjInsert(interp, nobj, "verStr", Jsi_ValueNewStringDup(interp, buf), 0);
        const char *cp = (ptr->loadFile?ptr->loadFile:"");
        Jsi_ObjInsert(interp, nobj, "loadFile", Jsi_ValueNewStringDup(interp, cp), 0);
        Jsi_Value *fval2, *fval = Jsi_NameLookup(interp, name);
        if (!fval || !Jsi_ValueIsFunction(interp, fval))
            fval = Jsi_ValueNewNull(interp);
        Jsi_ObjInsert(interp, nobj, "func", fval, 0);
        fval = ptr->popts.info;
        if (!fval) fval = interp->NullValue;
        if (!Jsi_ValueIsObjType(interp, fval, JSI_OT_FUNCTION))
            Jsi_ObjInsert(interp, nobj, "info", fval, 0);
        else {
            fval2 = Jsi_ValueNew1(interp);
            Jsi_RC rc = Jsi_FunctionInvoke(interp, fval, NULL, &fval2, NULL);
            if (rc != JSI_OK)
                Jsi_LogWarn("status call failed");
            Jsi_ObjInsert(interp, nobj, "info", fval2, 0);
            Jsi_DecrRefCount(interp, fval2);
        }
        fval = interp->NullValue;
        if (ptr->popts.spec && ptr->popts.data) {
            fval = Jsi_ValueNew1(interp);
            Jsi_OptionsConf(interp, ptr->popts.spec, ptr->popts.data, NULL, &fval, 0);
        }
        Jsi_ObjInsert(interp, nobj, "status", fval, 0);
        if (fval != interp->NullValue)
            Jsi_DecrRefCount(interp, fval);
        fval = Jsi_ValueNew1(interp);
        Jsi_OptionsConf(interp, jsiModuleOptions, &ptr->popts.modConf, NULL, &fval, 0);
        Jsi_ObjInsert(interp, nobj, "moduleOpts", fval, 0);
        Jsi_DecrRefCount(interp, fval);
        return JSI_OK;
    }
    return JSI_ERROR;
}