static void dbEvalSetColumnJSON(DbEvalContext *p, int iCol, Jsi_DString *dStr) {
    Jsi_Interp *interp = p->jdb->interp;
    char nbuf[200];
    sqlite3_stmt *pStmt = p->pPreStmt->pStmt;
    switch( sqlite3_column_type(pStmt, iCol) ) {
    case SQLITE_BLOB: {
        int bytes = sqlite3_column_bytes(pStmt, iCol);
        const char *zBlob = (char*)sqlite3_column_blob(pStmt, iCol);
        if( !zBlob ) {
            Jsi_DSAppend(dStr, "null", NULL);
            return;
        }
        Jsi_JSONQuote(interp, zBlob, bytes, dStr);
        return;
    }
    case SQLITE_INTEGER: {
        sqlite_int64 v = sqlite3_column_int64(pStmt, iCol);
        if (v==0 || v==1) {
            const char *dectyp = sqlite3_column_decltype(pStmt, iCol);
            if (dectyp && !Jsi_Strncasecmp(dectyp,"bool", 4)) {
                Jsi_DSAppend(dStr, (v?"true":"false"), NULL);
                return;
            }
        }
#ifdef __WIN32
        snprintf(nbuf, sizeof(nbuf), "%" PRId64, (Jsi_Wide)v);
#else
        snprintf(nbuf, sizeof(nbuf), "%lld", v);
#endif
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_FLOAT: {
        Jsi_NumberToString(interp, sqlite3_column_double(pStmt, iCol), nbuf, sizeof(nbuf));
        Jsi_DSAppend(dStr, nbuf, NULL);
        return;
    }
    case SQLITE_NULL: {
        Jsi_DSAppend(dStr, "null", NULL);
        return;
    }
    }
    const char *str = (char*)sqlite3_column_text(pStmt, iCol );
    if (!str)
        str = p->jdb->optPtr->nullvalue;
    Jsi_JSONQuote(interp, str?str:"", -1, dStr);
}