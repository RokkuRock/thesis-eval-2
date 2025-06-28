static void mdbEvalSetColumnJSON(MyDbEvalContext *p, int iCol, Jsi_DString *dStr) {
    Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];
    MysqlPrep *prep = p->prep;
    SqlFieldResults *field = prep->fieldResult+iCol;
    if (field->isnull) {
        Jsi_DSAppend(dStr, "null", NULL);
        return;
    }
    const char *zBlob = "";
    int bytes = 0;
    switch(field->jsiTypeMap) {
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
        case JSI_OPTION_DOUBLE: {
            Jsi_NumberToString(interp, field->buffer.vdouble, nbuf, sizeof(nbuf));
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
        case JSI_OPTION_STRING:
            zBlob = field->buffer.vstring;
        default:
        {
            if( !zBlob ) {
                Jsi_DSAppend(dStr, "null", NULL);
                return;
            }
            Jsi_JSONQuote(interp, zBlob, bytes, dStr);
            return;
        }
    }
}