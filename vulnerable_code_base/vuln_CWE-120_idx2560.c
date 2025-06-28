static void mdbEvalSetColumn(MyDbEvalContext *p, int iCol, Jsi_DString *dStr) {
    char nbuf[200];
    MysqlPrep *prep = p->prep;
    SqlFieldResults *field = prep->fieldResult+iCol;
    Jsi_Interp *interp = p->jdb->interp;
    if (field->isnull)
        return;
    switch(field->jsiTypeMap) {
        case JSI_OPTION_STRING: {
            int bytes = field->len;
            const char *zBlob = field->buffer.vstring;
            if( !zBlob ) {
                const char *nv = p->jdb->optPtr->nullvalue;
                Jsi_DSAppend(dStr, nv?nv:"null", NULL);
                return;
            }
            Jsi_DSAppendLen(dStr, zBlob, bytes);
            return;
        }
        case JSI_OPTION_BOOL: {
            snprintf(nbuf, sizeof(nbuf), "%s", field->buffer.vchar?"true":"false");
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_INT64: {
            snprintf(nbuf, sizeof(nbuf), "%lld", field->buffer.vlonglong);
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_TIME_D:
        case JSI_OPTION_TIME_W: {
            Jsi_Number jtime = mdbMyTimeToJS(&field->buffer.timestamp);
            Jsi_NumberToString(interp, jtime, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        case JSI_OPTION_DOUBLE: {
            Jsi_NumberToString(interp, field->buffer.vdouble, nbuf, sizeof(nbuf));
            Jsi_DSAppend(dStr, nbuf, NULL);
            return;
        }
        default:
            Jsi_LogWarn("unknown type: %d", field->jsiTypeMap);
    }
}