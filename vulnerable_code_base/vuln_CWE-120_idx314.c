static void jsiSqlFunc(sqlite3_context *context, int argc, sqlite3_value**argv) {
    SqlFunc *p = (SqlFunc*)sqlite3_user_data(context);
    int i;
    int rc;
    Jsi_Interp *interp = p->interp;
    Jsi_Value *vpargs, *itemsStatic[100], **items = itemsStatic, *ret;
    if (argc>100)
        items = (Jsi_Value**)Jsi_Calloc(argc, sizeof(Jsi_Value*));
    for(i=0; i<argc; i++) {
        items[i] = dbGetValueGet(interp, argv[i]);
    }
    vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, items, argc, 0));
    Jsi_IncrRefCount(interp, vpargs);
    ret = Jsi_ValueNew1(interp);
    rc = Jsi_FunctionInvoke(interp, p->tocall, vpargs, &ret, NULL);
    Jsi_DecrRefCount(interp, vpargs);
    if (items != itemsStatic)
        Jsi_Free(items);
    bool b;
    if( rc != JSI_OK) {
        char buf[250];
        snprintf(buf, sizeof(buf), "error in function: %.200s", p->zName);
        sqlite3_result_error(context, buf, -1);
    } else if (Jsi_ValueIsBoolean(interp, ret)) {
        Jsi_GetBoolFromValue(interp, ret, &b);
        sqlite3_result_int(context, b);
    } else if (Jsi_ValueIsNumber(interp, ret)) {
        Jsi_Number d;
        Jsi_GetNumberFromValue(interp, ret, &d);
        sqlite3_result_double(context, (double)d);
    } else {
        const char * data;
        if (!(data = Jsi_ValueGetStringLen(interp, ret, &i))) {
            data = Jsi_ValueToString(interp, ret, NULL);
            i = Jsi_Strlen(data);
        }
        sqlite3_result_text(context, (char *)data, i, SQLITE_TRANSIENT );
    }
    Jsi_DecrRefCount(interp, ret);
}