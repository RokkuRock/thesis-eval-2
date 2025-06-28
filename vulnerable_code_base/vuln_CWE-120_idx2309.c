static int jsi_DbQuery(Jsi_Db *jdb, Jsi_CDataDb *dbopts, const char *query)
{
    int k, cnt, erc = -1;
    Jsi_CDataDb statbinds[] = {{}, {}};
    if (!dbopts) dbopts = statbinds;
    OptionBind ob = {.binds = dbopts};
    Jsi_StructSpec *specPtr, *specs;
    Jsi_Interp *interp = jdb->interp;
    if (!query) query="";
    if (query[0]==';') {
        if (!dbExecCmd(jdb, query+1, &erc)) {
            Jsi_LogError("EXEC ERROR=\"%s\", SQL=\"%s\"", sqlite3_errmsg(jdb->db), query);
            return erc;
        }
        return 0;
    }
    const char *cPtr = Jsi_Strstr(query, " %s");
    if (!cPtr) cPtr = Jsi_Strstr(query, "\t%s");
    if (!dbopts) {
        Jsi_LogError("dbopts may not be null");
        return -1;
    }
    if (!dbopts[0].data) {
        Jsi_LogError("data may not be null");
        return -1;
    }
    if (!dbopts[0].sf) {
        Jsi_LogError("specs may not be null");
        return -1;
    }
    for (k=0; dbopts[k].sf; k++) {
        if (dbopts[k].arrSize>1 || k==0) {
            int scnt = 0;
            for (specPtr = dbopts[k].sf, scnt=0; specPtr->id>=JSI_OPTION_BOOL
                && specPtr->id < JSI_OPTION_END; specPtr++, scnt++) {
                if (specPtr->flags&JSI_OPT_DB_IGNORE)
                    continue;
                if (k==0) {
                    if (specPtr->flags&JSI_OPT_DB_ROWID) {
                        if (specPtr->id != JSI_OPTION_INT64) {
                            Jsi_LogError("rowid flag must be a wide field: %s", specPtr->name);
                            return -1;
                        }
                        ob.rowidPtr = specPtr;
                    }
                    if (specPtr->flags&JSI_OPT_DB_DIRTY) {
                        if (specPtr->id == JSI_OPTION_BOOL || specPtr->id == JSI_OPTION_INT) {
                            ob.dirtyPtr = specPtr;
                        } else {
                            Jsi_LogError("dirty flag must be a int/bool field: %s", specPtr->name);
                            return -1;
                        }
                    }
                }
            }
            if (k==0)
                ob.optLen = scnt;
            assert(specPtr->id == JSI_OPTION_END);
        }
        if (!dbopts[k].prefix) break;
    }
    specs = dbopts[0].sf;
    int structSize = specs[ob.optLen].size;
    if (dbopts->memClear || dbopts->memFree) {
        cnt = dbopts[0].arrSize;
        void *rec = dbopts[0].data, *prec = rec;
        void **recPtrPtr = NULL;
        if (dbopts->isPtr2) {
            recPtrPtr = (void**)rec;  
            rec = *recPtrPtr;
        }
        if (cnt<=0 && rec && dbopts->isPtr2) {
            for (cnt=0; ((void**)rec)[cnt]!=NULL; cnt++);
        }
        for (k=0; k<cnt; k++) {
            if (dbopts->isPtr2 || dbopts->isPtrs)
                prec = ((void**)rec)[k];
            else
                prec = (char*)rec + (k * structSize);
            if (!prec)
                continue;
            Jsi_OptionsFree(interp, (Jsi_OptionSpec*)specs, prec, 0);
            if (dbopts->isPtr2 || dbopts->isPtrs) {
                Jsi_Free(prec);
            }
        }
        if (recPtrPtr) {
            Jsi_Free(*recPtrPtr);
            *recPtrPtr = NULL;
        }
        if (query == NULL || query[0] == 0)
            return 0;
    }
    if (!Jsi_Strncasecmp(query, "SELECT", 6))
        return dbOptSelect(jdb, query, &ob, dbopts);
    DbEvalContext sEval = {};
    int insert = 0, replace = 0, update = 0;
    char nbuf[100], *bPtr;
#ifdef JSI_DB_DSTRING_SIZE
    JSI_DSTRING_VAR(dStr, JSI_DB_DSTRING_SIZE);
#else
    Jsi_DString sStr, *dStr = &sStr;
    Jsi_DSInit(dStr);
#endif
    if (dbopts->noCache)
        sEval.nocache = 1;
    if (dbEvalInit(interp, &sEval, jdb, NULL, dStr, 0, 0) != JSI_OK)
        return -1;
    int dataMax = dbopts[0].arrSize;
    cnt = 0;
    if (dataMax==0)
        dataMax = 1;
    char ch[2];
    ch[0] = dbopts[0].prefix;
    ch[1] = 0;
    if (!ch[0])
        ch[0] = ':';
    if ((update=(Jsi_Strncasecmp(query, "UPDATE", 6)==0))) {
        Jsi_DSAppendLen(dStr, query, cPtr?(cPtr-query):-1);
        if (cPtr) {
            Jsi_DSAppend(dStr, " ", NULL);
            int cidx = 0;
            int killf = (JSI_OPT_DB_IGNORE|JSI_OPT_READ_ONLY|JSI_OPT_INIT_ONLY);
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++, cidx++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr || (specPtr->flags&killf))
                    continue;
                const char *fname = specPtr->name;
                if (ch[0] == '?')
                    snprintf(bPtr=nbuf, sizeof(nbuf), "%d", cidx+1);
                else
                    bPtr = (char*)specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), "[", fname, "]=",
                    ch, bPtr, NULL);
                cnt++;
            }
            Jsi_DSAppend(dStr, cPtr+3, NULL);
        }
    } else if ((insert=(Jsi_Strncasecmp(query, "INSERT", 6)==0))
        || (replace=(Jsi_Strncasecmp(query, "REPLACE", 7)==0))) {
        Jsi_DSAppendLen(dStr, query, cPtr?(cPtr-query):-1);
        if (cPtr) {
            Jsi_DSAppend(dStr, " (", NULL);
            int killf = JSI_OPT_DB_IGNORE;
            if (replace)
                killf |= (JSI_OPT_READ_ONLY|JSI_OPT_INIT_ONLY);
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr || specPtr->flags&killf)
                    continue;
                const char *fname = specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), "[", fname, "]", NULL);
                cnt++;
            }
            Jsi_DSAppendLen(dStr,") VALUES(", -1);
            cnt = 0;
            int cidx = 0;
            for (specPtr = specs; specPtr->id != JSI_OPTION_END; specPtr++, cidx++) {
                if (specPtr == ob.rowidPtr || specPtr == ob.dirtyPtr
                    || specPtr->flags&killf)
                    continue;
                if (ch[0] == '?')
                    snprintf(bPtr=nbuf, sizeof(nbuf), "%d", cidx+1);
                else
                    bPtr = (char*)specPtr->name;
                Jsi_DSAppend(dStr, (cnt?",":""), ch, bPtr, NULL);
                cnt++;
            }
            Jsi_DSAppend(dStr,")", cPtr+3, NULL);
        }
    } else if (!Jsi_Strncasecmp(query, "DELETE", 6)) {
        Jsi_DSAppend(dStr, query, NULL);
    } else {
        Jsi_LogError("unrecognized query \"%s\": expected one of: SELECT, UPDATE, INSERT, REPLACE or DELETE", query);
        return -1;
    }
    sEval.zSql = Jsi_DSValue(dStr);
    if (jdb->echo && sEval.zSql)
        Jsi_LogInfo("SQL-ECHO: %s\n", sEval.zSql); 
    int rc, bindMax = -1, dataIdx = 0;
    cnt = 0;
    int ismodify = (replace||insert||update);
    int isnew = (replace||insert);
    int didBegin = 0;
    DbEvalContext *p = &sEval;
    rc = dbPrepareStmt(p->jdb, p->zSql, &p->zSql, &p->pPreStmt);
    if( rc!=JSI_OK ) return -1;
    if (dataMax>1 && !dbopts->noBegin) {
        didBegin = 1;
        if (!dbExecCmd(jdb, JSI_DBQUERY_BEGIN_STR, &erc))
            goto bail;
    }
    while (dataIdx<dataMax) {
        if (ismodify && ob.dirtyPtr && (dbopts->dirtyOnly)) {  
            void *rec = dbopts[0].data;
            if (dbopts->isPtrs || dbopts->isPtr2)
                rec = ((void**)rec)[dataIdx];
            else
                rec = (char*)rec + (dataIdx * structSize);
            char *ptr = (char*)rec + ob.dirtyPtr->offset;
            int isDirty = *(int*)ptr;
            int bit = 0;
            if (ob.dirtyPtr->id == JSI_OPTION_BOOL)
                bit = (uintptr_t)ob.dirtyPtr->data;
            if (!(isDirty&(1<<(bit)))) {
                dataIdx++;
                continue;
            }
            isDirty &= ~(1<<(bit));
            *(int*)ptr = isDirty;  
        }
        rc = dbBindOptionStmt(jdb, p->pPreStmt->pStmt, &ob, dataIdx, bindMax, dbopts);
        if( rc!=JSI_OK )
            goto bail;
        bindMax = 1;
        rc = dbEvalStepSub(p, (dataIdx>=dataMax), &erc);
        if (rc == JSI_ERROR)
            goto bail;
        cnt += sqlite3_changes(jdb->db);
        if (rc != JSI_OK && rc != JSI_BREAK)
            break;
        if (ob.rowidPtr && isnew) {
            void *rec = dbopts[0].data;
            if (dbopts->isPtrs || dbopts->isPtr2)
                rec = ((void**)rec)[dataIdx];
            else
                rec = (char*)rec + (dataIdx * structSize);
            char *ptr = (char*)rec + ob.rowidPtr->offset;
            *(Jsi_Wide*)ptr = sqlite3_last_insert_rowid(jdb->db);
        }
        dataIdx++;
    }
    if (didBegin && !dbExecCmd(jdb, JSI_DBQUERY_COMMIT_STR, &erc))
        rc = JSI_ERROR;
    dbEvalFinalize(&sEval);
    if( rc==JSI_BREAK ) {
        rc = JSI_OK;
    }
    return (rc==JSI_OK?cnt:erc);
bail:
    dbEvalFinalize(&sEval);
    if (didBegin)
        dbExecCmd(jdb, JSI_DBQUERY_ROLLBACK_STR, NULL);
    return erc;
}