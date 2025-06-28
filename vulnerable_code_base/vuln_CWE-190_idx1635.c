void sortCommandGeneric(client *c, int readonly) {
    list *operations;
    unsigned int outputlen = 0;
    int desc = 0, alpha = 0;
    long limit_start = 0, limit_count = -1, start, end;
    int j, dontsort = 0, vectorlen;
    int getop = 0;  
    int int_conversion_error = 0;
    int syntax_error = 0;
    robj *sortval, *sortby = NULL, *storekey = NULL;
    redisSortObject *vector;  
    int user_has_full_key_access = 0;  
    operations = listCreate();
    listSetFreeMethod(operations,zfree);
    j = 2;  
    user_has_full_key_access = ACLUserCheckCmdWithUnrestrictedKeyAccess(c->user, c->cmd, c->argv, c->argc, CMD_KEY_ACCESS);
    while(j < c->argc) {
        int leftargs = c->argc-j-1;
        if (!strcasecmp(c->argv[j]->ptr,"asc")) {
            desc = 0;
        } else if (!strcasecmp(c->argv[j]->ptr,"desc")) {
            desc = 1;
        } else if (!strcasecmp(c->argv[j]->ptr,"alpha")) {
            alpha = 1;
        } else if (!strcasecmp(c->argv[j]->ptr,"limit") && leftargs >= 2) {
            if ((getLongFromObjectOrReply(c, c->argv[j+1], &limit_start, NULL)
                 != C_OK) ||
                (getLongFromObjectOrReply(c, c->argv[j+2], &limit_count, NULL)
                 != C_OK))
            {
                syntax_error++;
                break;
            }
            j+=2;
        } else if (readonly == 0 && !strcasecmp(c->argv[j]->ptr,"store") && leftargs >= 1) {
            storekey = c->argv[j+1];
            j++;
        } else if (!strcasecmp(c->argv[j]->ptr,"by") && leftargs >= 1) {
            sortby = c->argv[j+1];
            if (strchr(c->argv[j+1]->ptr,'*') == NULL) {
                dontsort = 1;
            } else {
                if (server.cluster_enabled) {
                    addReplyError(c,"BY option of SORT denied in Cluster mode.");
                    syntax_error++;
                    break;
                }
                if (!user_has_full_key_access) {
                    addReplyError(c,"BY option of SORT denied due to insufficient ACL permissions.");
                    syntax_error++;
                    break;
                }
            }
            j++;
        } else if (!strcasecmp(c->argv[j]->ptr,"get") && leftargs >= 1) {
            if (server.cluster_enabled) {
                addReplyError(c,"GET option of SORT denied in Cluster mode.");
                syntax_error++;
                break;
            }
            if (!user_has_full_key_access) {
                addReplyError(c,"GET option of SORT denied due to insufficient ACL permissions.");
                syntax_error++;
                break;
            }
            listAddNodeTail(operations,createSortOperation(
                SORT_OP_GET,c->argv[j+1]));
            getop++;
            j++;
        } else {
            addReplyErrorObject(c,shared.syntaxerr);
            syntax_error++;
            break;
        }
        j++;
    }
    if (syntax_error) {
        listRelease(operations);
        return;
    }
    sortval = lookupKeyRead(c->db, c->argv[1]);
    if (sortval && sortval->type != OBJ_SET &&
                   sortval->type != OBJ_LIST &&
                   sortval->type != OBJ_ZSET)
    {
        listRelease(operations);
        addReplyErrorObject(c,shared.wrongtypeerr);
        return;
    }
    if (sortval)
        incrRefCount(sortval);
    else
        sortval = createQuicklistObject();
    if (dontsort &&
        sortval->type == OBJ_SET &&
        (storekey || c->flags & CLIENT_SCRIPT))
    {
        dontsort = 0;
        alpha = 1;
        sortby = NULL;
    }
    if (sortval->type == OBJ_ZSET)
        zsetConvert(sortval, OBJ_ENCODING_SKIPLIST);
    switch(sortval->type) {
    case OBJ_LIST: vectorlen = listTypeLength(sortval); break;
    case OBJ_SET: vectorlen =  setTypeSize(sortval); break;
    case OBJ_ZSET: vectorlen = dictSize(((zset*)sortval->ptr)->dict); break;
    default: vectorlen = 0; serverPanic("Bad SORT type");  
    }
    start = (limit_start < 0) ? 0 : limit_start;
    end = (limit_count < 0) ? vectorlen-1 : start+limit_count-1;
    if (start >= vectorlen) {
        start = vectorlen-1;
        end = vectorlen-2;
    }
    if (end >= vectorlen) end = vectorlen-1;
    if ((sortval->type == OBJ_ZSET || sortval->type == OBJ_LIST) &&
        dontsort &&
        (start != 0 || end != vectorlen-1))
    {
        vectorlen = end-start+1;
    }
    vector = zmalloc(sizeof(redisSortObject)*vectorlen);
    j = 0;
    if (sortval->type == OBJ_LIST && dontsort) {
        if (end >= start) {
            listTypeIterator *li;
            listTypeEntry entry;
            li = listTypeInitIterator(sortval,
                    desc ? (long)(listTypeLength(sortval) - start - 1) : start,
                    desc ? LIST_HEAD : LIST_TAIL);
            while(j < vectorlen && listTypeNext(li,&entry)) {
                vector[j].obj = listTypeGet(&entry);
                vector[j].u.score = 0;
                vector[j].u.cmpobj = NULL;
                j++;
            }
            listTypeReleaseIterator(li);
            end -= start;
            start = 0;
        }
    } else if (sortval->type == OBJ_LIST) {
        listTypeIterator *li = listTypeInitIterator(sortval,0,LIST_TAIL);
        listTypeEntry entry;
        while(listTypeNext(li,&entry)) {
            vector[j].obj = listTypeGet(&entry);
            vector[j].u.score = 0;
            vector[j].u.cmpobj = NULL;
            j++;
        }
        listTypeReleaseIterator(li);
    } else if (sortval->type == OBJ_SET) {
        setTypeIterator *si = setTypeInitIterator(sortval);
        sds sdsele;
        while((sdsele = setTypeNextObject(si)) != NULL) {
            vector[j].obj = createObject(OBJ_STRING,sdsele);
            vector[j].u.score = 0;
            vector[j].u.cmpobj = NULL;
            j++;
        }
        setTypeReleaseIterator(si);
    } else if (sortval->type == OBJ_ZSET && dontsort) {
        zset *zs = sortval->ptr;
        zskiplist *zsl = zs->zsl;
        zskiplistNode *ln;
        sds sdsele;
        int rangelen = vectorlen;
        if (desc) {
            long zsetlen = dictSize(((zset*)sortval->ptr)->dict);
            ln = zsl->tail;
            if (start > 0)
                ln = zslGetElementByRank(zsl,zsetlen-start);
        } else {
            ln = zsl->header->level[0].forward;
            if (start > 0)
                ln = zslGetElementByRank(zsl,start+1);
        }
        while(rangelen--) {
            serverAssertWithInfo(c,sortval,ln != NULL);
            sdsele = ln->ele;
            vector[j].obj = createStringObject(sdsele,sdslen(sdsele));
            vector[j].u.score = 0;
            vector[j].u.cmpobj = NULL;
            j++;
            ln = desc ? ln->backward : ln->level[0].forward;
        }
        end -= start;
        start = 0;
    } else if (sortval->type == OBJ_ZSET) {
        dict *set = ((zset*)sortval->ptr)->dict;
        dictIterator *di;
        dictEntry *setele;
        sds sdsele;
        di = dictGetIterator(set);
        while((setele = dictNext(di)) != NULL) {
            sdsele =  dictGetKey(setele);
            vector[j].obj = createStringObject(sdsele,sdslen(sdsele));
            vector[j].u.score = 0;
            vector[j].u.cmpobj = NULL;
            j++;
        }
        dictReleaseIterator(di);
    } else {
        serverPanic("Unknown type");
    }
    serverAssertWithInfo(c,sortval,j == vectorlen);
    if (!dontsort) {
        for (j = 0; j < vectorlen; j++) {
            robj *byval;
            if (sortby) {
                byval = lookupKeyByPattern(c->db,sortby,vector[j].obj);
                if (!byval) continue;
            } else {
                byval = vector[j].obj;
            }
            if (alpha) {
                if (sortby) vector[j].u.cmpobj = getDecodedObject(byval);
            } else {
                if (sdsEncodedObject(byval)) {
                    char *eptr;
                    vector[j].u.score = strtod(byval->ptr,&eptr);
                    if (eptr[0] != '\0' || errno == ERANGE ||
                        isnan(vector[j].u.score))
                    {
                        int_conversion_error = 1;
                    }
                } else if (byval->encoding == OBJ_ENCODING_INT) {
                    vector[j].u.score = (long)byval->ptr;
                } else {
                    serverAssertWithInfo(c,sortval,1 != 1);
                }
            }
            if (sortby) {
                decrRefCount(byval);
            }
        }
        server.sort_desc = desc;
        server.sort_alpha = alpha;
        server.sort_bypattern = sortby ? 1 : 0;
        server.sort_store = storekey ? 1 : 0;
        if (sortby && (start != 0 || end != vectorlen-1))
            pqsort(vector,vectorlen,sizeof(redisSortObject),sortCompare, start,end);
        else
            qsort(vector,vectorlen,sizeof(redisSortObject),sortCompare);
    }
    outputlen = getop ? getop*(end-start+1) : end-start+1;
    if (int_conversion_error) {
        addReplyError(c,"One or more scores can't be converted into double");
    } else if (storekey == NULL) {
        addReplyArrayLen(c,outputlen);
        for (j = start; j <= end; j++) {
            listNode *ln;
            listIter li;
            if (!getop) addReplyBulk(c,vector[j].obj);
            listRewind(operations,&li);
            while((ln = listNext(&li))) {
                redisSortOperation *sop = ln->value;
                robj *val = lookupKeyByPattern(c->db,sop->pattern,
                                               vector[j].obj);
                if (sop->type == SORT_OP_GET) {
                    if (!val) {
                        addReplyNull(c);
                    } else {
                        addReplyBulk(c,val);
                        decrRefCount(val);
                    }
                } else {
                    serverAssertWithInfo(c,sortval,sop->type == SORT_OP_GET);
                }
            }
        }
    } else {
        robj *sobj = createQuicklistObject();
        for (j = start; j <= end; j++) {
            listNode *ln;
            listIter li;
            if (!getop) {
                listTypePush(sobj,vector[j].obj,LIST_TAIL);
            } else {
                listRewind(operations,&li);
                while((ln = listNext(&li))) {
                    redisSortOperation *sop = ln->value;
                    robj *val = lookupKeyByPattern(c->db,sop->pattern,
                                                   vector[j].obj);
                    if (sop->type == SORT_OP_GET) {
                        if (!val) val = createStringObject("",0);
                        listTypePush(sobj,val,LIST_TAIL);
                        decrRefCount(val);
                    } else {
                        serverAssertWithInfo(c,sortval,sop->type == SORT_OP_GET);
                    }
                }
            }
        }
        if (outputlen) {
            listTypeTryConversion(sobj,LIST_CONV_AUTO,NULL,NULL);
            setKey(c,c->db,storekey,sobj,0);
            notifyKeyspaceEvent(NOTIFY_LIST,"sortstore",storekey,
                                c->db->id);
            server.dirty += outputlen;
        } else if (dbDelete(c->db,storekey)) {
            signalModifiedKey(c,c->db,storekey);
            notifyKeyspaceEvent(NOTIFY_GENERIC,"del",storekey,c->db->id);
            server.dirty++;
        }
        decrRefCount(sobj);
        addReplyLongLong(c,outputlen);
    }
    for (j = 0; j < vectorlen; j++)
        decrRefCount(vector[j].obj);
    decrRefCount(sortval);
    listRelease(operations);
    for (j = 0; j < vectorlen; j++) {
        if (alpha && vector[j].u.cmpobj)
            decrRefCount(vector[j].u.cmpobj);
    }
    zfree(vector);
}