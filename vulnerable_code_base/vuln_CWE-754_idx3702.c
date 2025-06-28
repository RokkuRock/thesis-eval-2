void sqlite3Pragma(
  Parse *pParse, 
  Token *pId1,         
  Token *pId2,         
  Token *pValue,       
  int minusFlag        
){
  char *zLeft = 0;        
  char *zRight = 0;       
  const char *zDb = 0;    
  Token *pId;             
  char *aFcntl[4];        
  int iDb;                
  int rc;                       
  sqlite3 *db = pParse->db;     
  Db *pDb;                      
  Vdbe *v = sqlite3GetVdbe(pParse);   
  const PragmaName *pPragma;    
  if( v==0 ) return;
  sqlite3VdbeRunOnlyOnce(v);
  pParse->nMem = 2;
  iDb = sqlite3TwoPartName(pParse, pId1, pId2, &pId);
  if( iDb<0 ) return;
  pDb = &db->aDb[iDb];
  if( iDb==1 && sqlite3OpenTempDatabase(pParse) ){
    return;
  }
  zLeft = sqlite3NameFromToken(db, pId);
  if( !zLeft ) return;
  if( minusFlag ){
    zRight = sqlite3MPrintf(db, "-%T", pValue);
  }else{
    zRight = sqlite3NameFromToken(db, pValue);
  }
  assert( pId2 );
  zDb = pId2->n>0 ? pDb->zDbSName : 0;
  if( sqlite3AuthCheck(pParse, SQLITE_PRAGMA, zLeft, zRight, zDb) ){
    goto pragma_out;
  }
  aFcntl[0] = 0;
  aFcntl[1] = zLeft;
  aFcntl[2] = zRight;
  aFcntl[3] = 0;
  db->busyHandler.nBusy = 0;
  rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_PRAGMA, (void*)aFcntl);
  if( rc==SQLITE_OK ){
    sqlite3VdbeSetNumCols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, aFcntl[0], SQLITE_TRANSIENT);
    returnSingleText(v, aFcntl[0]);
    sqlite3_free(aFcntl[0]);
    goto pragma_out;
  }
  if( rc!=SQLITE_NOTFOUND ){
    if( aFcntl[0] ){
      sqlite3ErrorMsg(pParse, "%s", aFcntl[0]);
      sqlite3_free(aFcntl[0]);
    }
    pParse->nErr++;
    pParse->rc = rc;
    goto pragma_out;
  }
  pPragma = pragmaLocate(zLeft);
  if( pPragma==0 ) goto pragma_out;
  if( (pPragma->mPragFlg & PragFlg_NeedSchema)!=0 ){
    if( sqlite3ReadSchema(pParse) ) goto pragma_out;
  }
  if( (pPragma->mPragFlg & PragFlg_NoColumns)==0 
   && ((pPragma->mPragFlg & PragFlg_NoColumns1)==0 || zRight==0)
  ){
    setPragmaResultColumnNames(v, pPragma);
  }
  switch( pPragma->ePragTyp ){
#if !defined(SQLITE_OMIT_PAGER_PRAGMAS) && !defined(SQLITE_OMIT_DEPRECATED)
  case PragTyp_DEFAULT_CACHE_SIZE: {
    static const int iLn = VDBE_OFFSET_LINENO(2);
    static const VdbeOpList getCacheSize[] = {
      { OP_Transaction, 0, 0,        0},                          
      { OP_ReadCookie,  0, 1,        BTREE_DEFAULT_CACHE_SIZE},   
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 2,        0},
      { OP_Subtract,    1, 2,        1},
      { OP_IfPos,       1, 8,        0},
      { OP_Integer,     0, 1,        0},                          
      { OP_Noop,        0, 0,        0},
      { OP_ResultRow,   1, 1,        0},
    };
    VdbeOp *aOp;
    sqlite3VdbeUsesBtree(v, iDb);
    if( !zRight ){
      pParse->nMem += 2;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(getCacheSize));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(getCacheSize), getCacheSize, iLn);
      if( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[6].p1 = SQLITE_DEFAULT_CACHE_SIZE;
    }else{
      int size = sqlite3AbsInt32(sqlite3Atoi(zRight));
      sqlite3BeginWriteOperation(pParse, 0, iDb);
      sqlite3VdbeAddOp3(v, OP_SetCookie, iDb, BTREE_DEFAULT_CACHE_SIZE, size);
      assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    break;
  }
#endif  
#if !defined(SQLITE_OMIT_PAGER_PRAGMAS)
  case PragTyp_PAGE_SIZE: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    if( !zRight ){
      int size = ALWAYS(pBt) ? sqlite3BtreeGetPageSize(pBt) : 0;
      returnSingleInt(v, size);
    }else{
      db->nextPagesize = sqlite3Atoi(zRight);
      if( SQLITE_NOMEM==sqlite3BtreeSetPageSize(pBt, db->nextPagesize,-1,0) ){
        sqlite3OomFault(db);
      }
    }
    break;
  }
  case PragTyp_SECURE_DELETE: {
    Btree *pBt = pDb->pBt;
    int b = -1;
    assert( pBt!=0 );
    if( zRight ){
      if( sqlite3_stricmp(zRight, "fast")==0 ){
        b = 2;
      }else{
        b = sqlite3GetBoolean(zRight, 0);
      }
    }
    if( pId2->n==0 && b>=0 ){
      int ii;
      for(ii=0; ii<db->nDb; ii++){
        sqlite3BtreeSecureDelete(db->aDb[ii].pBt, b);
      }
    }
    b = sqlite3BtreeSecureDelete(pBt, b);
    returnSingleInt(v, b);
    break;
  }
  case PragTyp_PAGE_COUNT: {
    int iReg;
    sqlite3CodeVerifySchema(pParse, iDb);
    iReg = ++pParse->nMem;
    if( sqlite3Tolower(zLeft[0])=='p' ){
      sqlite3VdbeAddOp2(v, OP_Pagecount, iDb, iReg);
    }else{
      sqlite3VdbeAddOp3(v, OP_MaxPgcnt, iDb, iReg, 
                        sqlite3AbsInt32(sqlite3Atoi(zRight)));
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, iReg, 1);
    break;
  }
  case PragTyp_LOCKING_MODE: {
    const char *zRet = "normal";
    int eMode = getLockingMode(zRight);
    if( pId2->n==0 && eMode==PAGER_LOCKINGMODE_QUERY ){
      eMode = db->dfltLockMode;
    }else{
      Pager *pPager;
      if( pId2->n==0 ){
        int ii;
        assert(pDb==&db->aDb[0]);
        for(ii=2; ii<db->nDb; ii++){
          pPager = sqlite3BtreePager(db->aDb[ii].pBt);
          sqlite3PagerLockingMode(pPager, eMode);
        }
        db->dfltLockMode = (u8)eMode;
      }
      pPager = sqlite3BtreePager(pDb->pBt);
      eMode = sqlite3PagerLockingMode(pPager, eMode);
    }
    assert( eMode==PAGER_LOCKINGMODE_NORMAL
            || eMode==PAGER_LOCKINGMODE_EXCLUSIVE );
    if( eMode==PAGER_LOCKINGMODE_EXCLUSIVE ){
      zRet = "exclusive";
    }
    returnSingleText(v, zRet);
    break;
  }
  case PragTyp_JOURNAL_MODE: {
    int eMode;         
    int ii;            
    if( zRight==0 ){
      eMode = PAGER_JOURNALMODE_QUERY;
    }else{
      const char *zMode;
      int n = sqlite3Strlen30(zRight);
      for(eMode=0; (zMode = sqlite3JournalModename(eMode))!=0; eMode++){
        if( sqlite3StrNICmp(zRight, zMode, n)==0 ) break;
      }
      if( !zMode ){
        eMode = PAGER_JOURNALMODE_QUERY;
      }
      if( eMode==PAGER_JOURNALMODE_OFF && (db->flags & SQLITE_Defensive)!=0 ){
        eMode = PAGER_JOURNALMODE_QUERY;
      }
    }
    if( eMode==PAGER_JOURNALMODE_QUERY && pId2->n==0 ){
      iDb = 0;
      pId2->n = 1;
    }
    for(ii=db->nDb-1; ii>=0; ii--){
      if( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
        sqlite3VdbeUsesBtree(v, ii);
        sqlite3VdbeAddOp3(v, OP_JournalMode, ii, 1, eMode);
      }
    }
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    break;
  }
  case PragTyp_JOURNAL_SIZE_LIMIT: {
    Pager *pPager = sqlite3BtreePager(pDb->pBt);
    i64 iLimit = -2;
    if( zRight ){
      sqlite3DecOrHexToI64(zRight, &iLimit);
      if( iLimit<-1 ) iLimit = -1;
    }
    iLimit = sqlite3PagerJournalSizeLimit(pPager, iLimit);
    returnSingleInt(v, iLimit);
    break;
  }
#endif  
#ifndef SQLITE_OMIT_AUTOVACUUM
  case PragTyp_AUTO_VACUUM: {
    Btree *pBt = pDb->pBt;
    assert( pBt!=0 );
    if( !zRight ){
      returnSingleInt(v, sqlite3BtreeGetAutoVacuum(pBt));
    }else{
      int eAuto = getAutoVacuum(zRight);
      assert( eAuto>=0 && eAuto<=2 );
      db->nextAutovac = (u8)eAuto;
      rc = sqlite3BtreeSetAutoVacuum(pBt, eAuto);
      if( rc==SQLITE_OK && (eAuto==1 || eAuto==2) ){
        static const int iLn = VDBE_OFFSET_LINENO(2);
        static const VdbeOpList setMeta6[] = {
          { OP_Transaction,    0,         1,                 0},     
          { OP_ReadCookie,     0,         1,         BTREE_LARGEST_ROOT_PAGE},
          { OP_If,             1,         0,                 0},     
          { OP_Halt,           SQLITE_OK, OE_Abort,          0},     
          { OP_SetCookie,      0,         BTREE_INCR_VACUUM, 0},     
        };
        VdbeOp *aOp;
        int iAddr = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setMeta6));
        aOp = sqlite3VdbeAddOpList(v, ArraySize(setMeta6), setMeta6, iLn);
        if( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
        aOp[0].p1 = iDb;
        aOp[1].p1 = iDb;
        aOp[2].p2 = iAddr+4;
        aOp[4].p1 = iDb;
        aOp[4].p3 = eAuto - 1;
        sqlite3VdbeUsesBtree(v, iDb);
      }
    }
    break;
  }
#endif
#ifndef SQLITE_OMIT_AUTOVACUUM
  case PragTyp_INCREMENTAL_VACUUM: {
    int iLimit, addr;
    if( zRight==0 || !sqlite3GetInt32(zRight, &iLimit) || iLimit<=0 ){
      iLimit = 0x7fffffff;
    }
    sqlite3BeginWriteOperation(pParse, 0, iDb);
    sqlite3VdbeAddOp2(v, OP_Integer, iLimit, 1);
    addr = sqlite3VdbeAddOp1(v, OP_IncrVacuum, iDb); VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_ResultRow, 1);
    sqlite3VdbeAddOp2(v, OP_AddImm, 1, -1);
    sqlite3VdbeAddOp2(v, OP_IfPos, 1, addr); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr);
    break;
  }
#endif
#ifndef SQLITE_OMIT_PAGER_PRAGMAS
  case PragTyp_CACHE_SIZE: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    if( !zRight ){
      returnSingleInt(v, pDb->pSchema->cache_size);
    }else{
      int size = sqlite3Atoi(zRight);
      pDb->pSchema->cache_size = size;
      sqlite3BtreeSetCacheSize(pDb->pBt, pDb->pSchema->cache_size);
    }
    break;
  }
  case PragTyp_CACHE_SPILL: {
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    if( !zRight ){
      returnSingleInt(v,
         (db->flags & SQLITE_CacheSpill)==0 ? 0 : 
            sqlite3BtreeSetSpillSize(pDb->pBt,0));
    }else{
      int size = 1;
      if( sqlite3GetInt32(zRight, &size) ){
        sqlite3BtreeSetSpillSize(pDb->pBt, size);
      }
      if( sqlite3GetBoolean(zRight, size!=0) ){
        db->flags |= SQLITE_CacheSpill;
      }else{
        db->flags &= ~(u64)SQLITE_CacheSpill;
      }
      setAllPagerFlags(db);
    }
    break;
  }
  case PragTyp_MMAP_SIZE: {
    sqlite3_int64 sz;
#if SQLITE_MAX_MMAP_SIZE>0
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    if( zRight ){
      int ii;
      sqlite3DecOrHexToI64(zRight, &sz);
      if( sz<0 ) sz = sqlite3GlobalConfig.szMmap;
      if( pId2->n==0 ) db->szMmap = sz;
      for(ii=db->nDb-1; ii>=0; ii--){
        if( db->aDb[ii].pBt && (ii==iDb || pId2->n==0) ){
          sqlite3BtreeSetMmapLimit(db->aDb[ii].pBt, sz);
        }
      }
    }
    sz = -1;
    rc = sqlite3_file_control(db, zDb, SQLITE_FCNTL_MMAP_SIZE, &sz);
#else
    sz = 0;
    rc = SQLITE_OK;
#endif
    if( rc==SQLITE_OK ){
      returnSingleInt(v, sz);
    }else if( rc!=SQLITE_NOTFOUND ){
      pParse->nErr++;
      pParse->rc = rc;
    }
    break;
  }
  case PragTyp_TEMP_STORE: {
    if( !zRight ){
      returnSingleInt(v, db->temp_store);
    }else{
      changeTempStorage(pParse, zRight);
    }
    break;
  }
  case PragTyp_TEMP_STORE_DIRECTORY: {
    if( !zRight ){
      returnSingleText(v, sqlite3_temp_directory);
    }else{
#ifndef SQLITE_OMIT_WSD
      if( zRight[0] ){
        int res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        if( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          goto pragma_out;
        }
      }
      if( SQLITE_TEMP_STORE==0
       || (SQLITE_TEMP_STORE==1 && db->temp_store<=1)
       || (SQLITE_TEMP_STORE==2 && db->temp_store==1)
      ){
        invalidateTempStorage(pParse);
      }
      sqlite3_free(sqlite3_temp_directory);
      if( zRight[0] ){
        sqlite3_temp_directory = sqlite3_mprintf("%s", zRight);
      }else{
        sqlite3_temp_directory = 0;
      }
#endif  
    }
    break;
  }
#if SQLITE_OS_WIN
  case PragTyp_DATA_STORE_DIRECTORY: {
    if( !zRight ){
      returnSingleText(v, sqlite3_data_directory);
    }else{
#ifndef SQLITE_OMIT_WSD
      if( zRight[0] ){
        int res;
        rc = sqlite3OsAccess(db->pVfs, zRight, SQLITE_ACCESS_READWRITE, &res);
        if( rc!=SQLITE_OK || res==0 ){
          sqlite3ErrorMsg(pParse, "not a writable directory");
          goto pragma_out;
        }
      }
      sqlite3_free(sqlite3_data_directory);
      if( zRight[0] ){
        sqlite3_data_directory = sqlite3_mprintf("%s", zRight);
      }else{
        sqlite3_data_directory = 0;
      }
#endif  
    }
    break;
  }
#endif
#if SQLITE_ENABLE_LOCKING_STYLE
  case PragTyp_LOCK_PROXY_FILE: {
    if( !zRight ){
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      char *proxy_file_path = NULL;
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      sqlite3OsFileControlHint(pFile, SQLITE_GET_LOCKPROXYFILE, 
                           &proxy_file_path);
      returnSingleText(v, proxy_file_path);
    }else{
      Pager *pPager = sqlite3BtreePager(pDb->pBt);
      sqlite3_file *pFile = sqlite3PagerFile(pPager);
      int res;
      if( zRight[0] ){
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE, 
                                     zRight);
      } else {
        res=sqlite3OsFileControl(pFile, SQLITE_SET_LOCKPROXYFILE, 
                                     NULL);
      }
      if( res!=SQLITE_OK ){
        sqlite3ErrorMsg(pParse, "failed to set lock proxy file");
        goto pragma_out;
      }
    }
    break;
  }
#endif        
  case PragTyp_SYNCHRONOUS: {
    if( !zRight ){
      returnSingleInt(v, pDb->safety_level-1);
    }else{
      if( !db->autoCommit ){
        sqlite3ErrorMsg(pParse, 
            "Safety level may not be changed inside a transaction");
      }else if( iDb!=1 ){
        int iLevel = (getSafetyLevel(zRight,0,1)+1) & PAGER_SYNCHRONOUS_MASK;
        if( iLevel==0 ) iLevel = 1;
        pDb->safety_level = iLevel;
        pDb->bSyncSet = 1;
        setAllPagerFlags(db);
      }
    }
    break;
  }
#endif  
#ifndef SQLITE_OMIT_FLAG_PRAGMAS
  case PragTyp_FLAG: {
    if( zRight==0 ){
      setPragmaResultColumnNames(v, pPragma);
      returnSingleInt(v, (db->flags & pPragma->iArg)!=0 );
    }else{
      u64 mask = pPragma->iArg;     
      if( db->autoCommit==0 ){
        mask &= ~(SQLITE_ForeignKeys);
      }
#if SQLITE_USER_AUTHENTICATION
      if( db->auth.authLevel==UAUTH_User ){
        mask &= ~(SQLITE_WriteSchema);
      }
#endif
      if( sqlite3GetBoolean(zRight, 0) ){
        db->flags |= mask;
      }else{
        db->flags &= ~mask;
        if( mask==SQLITE_DeferFKs ) db->nDeferredImmCons = 0;
      }
      sqlite3VdbeAddOp0(v, OP_Expire);
      setAllPagerFlags(db);
    }
    break;
  }
#endif  
#ifndef SQLITE_OMIT_SCHEMA_PRAGMAS
  case PragTyp_TABLE_INFO: if( zRight ){
    Table *pTab;
    pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
    if( pTab ){
      int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      int i, k;
      int nHidden = 0;
      Column *pCol;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      pParse->nMem = 7;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      sqlite3ViewGetColumnNames(pParse, pTab);
      for(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
        int isHidden = 0;
        if( pCol->colFlags & COLFLAG_NOINSERT ){
          if( pPragma->iArg==0 ){
            nHidden++;
            continue;
          }
          if( pCol->colFlags & COLFLAG_VIRTUAL ){
            isHidden = 2;   
          }else if( pCol->colFlags & COLFLAG_STORED ){
            isHidden = 3;   
          }else{ assert( pCol->colFlags & COLFLAG_HIDDEN );
            isHidden = 1;   
          }
        }
        if( (pCol->colFlags & COLFLAG_PRIMKEY)==0 ){
          k = 0;
        }else if( pPk==0 ){
          k = 1;
        }else{
          for(k=1; k<=pTab->nCol && pPk->aiColumn[k-1]!=i; k++){}
        }
        assert( pCol->pDflt==0 || pCol->pDflt->op==TK_SPAN || isHidden>=2 );
        sqlite3VdbeMultiLoad(v, 1, pPragma->iArg ? "issisii" : "issisi",
               i-nHidden,
               pCol->zName,
               sqlite3ColumnType(pCol,""),
               pCol->notNull ? 1 : 0,
               pCol->pDflt && isHidden<2 ? pCol->pDflt->u.zToken : 0,
               k,
               isHidden);
      }
    }
  }
  break;
#ifdef SQLITE_DEBUG
  case PragTyp_STATS: {
    Index *pIdx;
    HashElem *i;
    pParse->nMem = 5;
    sqlite3CodeVerifySchema(pParse, iDb);
    for(i=sqliteHashFirst(&pDb->pSchema->tblHash); i; i=sqliteHashNext(i)){
      Table *pTab = sqliteHashData(i);
      sqlite3VdbeMultiLoad(v, 1, "ssiii",
           pTab->zName,
           0,
           pTab->szTabRow,
           pTab->nRowLogEst,
           pTab->tabFlags);
      for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
        sqlite3VdbeMultiLoad(v, 2, "siiiX",
           pIdx->zName,
           pIdx->szIdxRow,
           pIdx->aiRowLogEst[0],
           pIdx->hasStat1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 5);
      }
    }
  }
  break;
#endif
  case PragTyp_INDEX_INFO: if( zRight ){
    Index *pIdx;
    Table *pTab;
    pIdx = sqlite3FindIndex(db, zRight, zDb);
    if( pIdx==0 ){
      pTab = sqlite3LocateTable(pParse, LOCATE_NOERR, zRight, zDb);
      if( pTab && !HasRowid(pTab) ){
        pIdx = sqlite3PrimaryKeyIndex(pTab);
      }
    }
    if( pIdx ){
      int iIdxDb = sqlite3SchemaToIndex(db, pIdx->pSchema);
      int i;
      int mx;
      if( pPragma->iArg ){
        mx = pIdx->nColumn;
        pParse->nMem = 6;
      }else{
        mx = pIdx->nKeyCol;
        pParse->nMem = 3;
      }
      pTab = pIdx->pTable;
      sqlite3CodeVerifySchema(pParse, iIdxDb);
      assert( pParse->nMem<=pPragma->nPragCName );
      for(i=0; i<mx; i++){
        i16 cnum = pIdx->aiColumn[i];
        sqlite3VdbeMultiLoad(v, 1, "iisX", i, cnum,
                             cnum<0 ? 0 : pTab->aCol[cnum].zName);
        if( pPragma->iArg ){
          sqlite3VdbeMultiLoad(v, 4, "isiX",
            pIdx->aSortOrder[i],
            pIdx->azColl[i],
            i<pIdx->nKeyCol);
        }
        sqlite3VdbeAddOp2(v, OP_ResultRow, 1, pParse->nMem);
      }
    }
  }
  break;
  case PragTyp_INDEX_LIST: if( zRight ){
    Index *pIdx;
    Table *pTab;
    int i;
    pTab = sqlite3FindTable(db, zRight, zDb);
    if( pTab ){
      int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      pParse->nMem = 5;
      sqlite3CodeVerifySchema(pParse, iTabDb);
      for(pIdx=pTab->pIndex, i=0; pIdx; pIdx=pIdx->pNext, i++){
        const char *azOrigin[] = { "c", "u", "pk" };
        sqlite3VdbeMultiLoad(v, 1, "isisi",
           i,
           pIdx->zName,
           IsUniqueIndex(pIdx),
           azOrigin[pIdx->idxType],
           pIdx->pPartIdxWhere!=0);
      }
    }
  }
  break;
  case PragTyp_DATABASE_LIST: {
    int i;
    pParse->nMem = 3;
    for(i=0; i<db->nDb; i++){
      if( db->aDb[i].pBt==0 ) continue;
      assert( db->aDb[i].zDbSName!=0 );
      sqlite3VdbeMultiLoad(v, 1, "iss",
         i,
         db->aDb[i].zDbSName,
         sqlite3BtreeGetFilename(db->aDb[i].pBt));
    }
  }
  break;
  case PragTyp_COLLATION_LIST: {
    int i = 0;
    HashElem *p;
    pParse->nMem = 2;
    for(p=sqliteHashFirst(&db->aCollSeq); p; p=sqliteHashNext(p)){
      CollSeq *pColl = (CollSeq *)sqliteHashData(p);
      sqlite3VdbeMultiLoad(v, 1, "is", i++, pColl->zName);
    }
  }
  break;
#ifndef SQLITE_OMIT_INTROSPECTION_PRAGMAS
  case PragTyp_FUNCTION_LIST: {
    int i;
    HashElem *j;
    FuncDef *p;
    pParse->nMem = 2;
    for(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      for(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash ){
        if( p->funcFlags & SQLITE_FUNC_INTERNAL ) continue;
        sqlite3VdbeMultiLoad(v, 1, "si", p->zName, 1);
      }
    }
    for(j=sqliteHashFirst(&db->aFunc); j; j=sqliteHashNext(j)){
      p = (FuncDef*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "si", p->zName, 0);
    }
  }
  break;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  case PragTyp_MODULE_LIST: {
    HashElem *j;
    pParse->nMem = 1;
    for(j=sqliteHashFirst(&db->aModule); j; j=sqliteHashNext(j)){
      Module *pMod = (Module*)sqliteHashData(j);
      sqlite3VdbeMultiLoad(v, 1, "s", pMod->zName);
    }
  }
  break;
#endif  
  case PragTyp_PRAGMA_LIST: {
    int i;
    for(i=0; i<ArraySize(aPragmaName); i++){
      sqlite3VdbeMultiLoad(v, 1, "s", aPragmaName[i].zName);
    }
  }
  break;
#endif  
#endif  
#ifndef SQLITE_OMIT_FOREIGN_KEY
  case PragTyp_FOREIGN_KEY_LIST: if( zRight ){
    FKey *pFK;
    Table *pTab;
    pTab = sqlite3FindTable(db, zRight, zDb);
    if( pTab ){
      pFK = pTab->pFKey;
      if( pFK ){
        int iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
        int i = 0; 
        pParse->nMem = 8;
        sqlite3CodeVerifySchema(pParse, iTabDb);
        while(pFK){
          int j;
          for(j=0; j<pFK->nCol; j++){
            sqlite3VdbeMultiLoad(v, 1, "iissssss",
                   i,
                   j,
                   pFK->zTo,
                   pTab->aCol[pFK->aCol[j].iFrom].zName,
                   pFK->aCol[j].zCol,
                   actionName(pFK->aAction[1]),   
                   actionName(pFK->aAction[0]),   
                   "NONE");
          }
          ++i;
          pFK = pFK->pNextFrom;
        }
      }
    }
  }
  break;
#endif  
#ifndef SQLITE_OMIT_FOREIGN_KEY
#ifndef SQLITE_OMIT_TRIGGER
  case PragTyp_FOREIGN_KEY_CHECK: {
    FKey *pFK;              
    Table *pTab;            
    Table *pParent;         
    Index *pIdx;            
    int i;                  
    int j;                  
    HashElem *k;            
    int x;                  
    int regResult;          
    int regKey;             
    int regRow;             
    int addrTop;            
    int addrOk;             
    int *aiCols;            
    regResult = pParse->nMem+1;
    pParse->nMem += 4;
    regKey = ++pParse->nMem;
    regRow = ++pParse->nMem;
    k = sqliteHashFirst(&db->aDb[iDb].pSchema->tblHash);
    while( k ){
      int iTabDb;
      if( zRight ){
        pTab = sqlite3LocateTable(pParse, 0, zRight, zDb);
        k = 0;
      }else{
        pTab = (Table*)sqliteHashData(k);
        k = sqliteHashNext(k);
      }
      if( pTab==0 || pTab->pFKey==0 ) continue;
      iTabDb = sqlite3SchemaToIndex(db, pTab->pSchema);
      sqlite3CodeVerifySchema(pParse, iTabDb);
      sqlite3TableLock(pParse, iTabDb, pTab->tnum, 0, pTab->zName);
      if( pTab->nCol+regRow>pParse->nMem ) pParse->nMem = pTab->nCol + regRow;
      sqlite3OpenTable(pParse, 0, iTabDb, pTab, OP_OpenRead);
      sqlite3VdbeLoadString(v, regResult, pTab->zName);
      for(i=1, pFK=pTab->pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        if( pParent==0 ) continue;
        pIdx = 0;
        sqlite3TableLock(pParse, iTabDb, pParent->tnum, 0, pParent->zName);
        x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, 0);
        if( x==0 ){
          if( pIdx==0 ){
            sqlite3OpenTable(pParse, i, iTabDb, pParent, OP_OpenRead);
          }else{
            sqlite3VdbeAddOp3(v, OP_OpenRead, i, pIdx->tnum, iTabDb);
            sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
          }
        }else{
          k = 0;
          break;
        }
      }
      assert( pParse->nErr>0 || pFK==0 );
      if( pFK ) break;
      if( pParse->nTab<i ) pParse->nTab = i;
      addrTop = sqlite3VdbeAddOp1(v, OP_Rewind, 0); VdbeCoverage(v);
      for(i=1, pFK=pTab->pFKey; pFK; i++, pFK=pFK->pNextFrom){
        pParent = sqlite3FindTable(db, pFK->zTo, zDb);
        pIdx = 0;
        aiCols = 0;
        if( pParent ){
          x = sqlite3FkLocateIndex(pParse, pParent, pFK, &pIdx, &aiCols);
          assert( x==0 );
        }
        addrOk = sqlite3VdbeMakeLabel(pParse);
        for(j=0; j<pFK->nCol; j++){
          int iCol = aiCols ? aiCols[j] : pFK->aCol[j].iFrom;
          sqlite3ExprCodeGetColumnOfTable(v, pTab, 0, iCol, regRow+j);
          sqlite3VdbeAddOp2(v, OP_IsNull, regRow+j, addrOk); VdbeCoverage(v);
        }
        if( pIdx ){
          sqlite3VdbeAddOp4(v, OP_MakeRecord, regRow, pFK->nCol, regKey,
              sqlite3IndexAffinityStr(db,pIdx), pFK->nCol);
          sqlite3VdbeAddOp4Int(v, OP_Found, i, addrOk, regKey, 0);
          VdbeCoverage(v);
        }else if( pParent ){
          int jmp = sqlite3VdbeCurrentAddr(v)+2;
          sqlite3VdbeAddOp3(v, OP_SeekRowid, i, jmp, regRow); VdbeCoverage(v);
          sqlite3VdbeGoto(v, addrOk);
          assert( pFK->nCol==1 );
        }
        if( HasRowid(pTab) ){
          sqlite3VdbeAddOp2(v, OP_Rowid, 0, regResult+1);
        }else{
          sqlite3VdbeAddOp2(v, OP_Null, 0, regResult+1);
        }
        sqlite3VdbeMultiLoad(v, regResult+2, "siX", pFK->zTo, i-1);
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, 4);
        sqlite3VdbeResolveLabel(v, addrOk);
        sqlite3DbFree(db, aiCols);
      }
      sqlite3VdbeAddOp2(v, OP_Next, 0, addrTop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrTop);
    }
  }
  break;
#endif  
#endif  
#ifndef SQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA
  case PragTyp_CASE_SENSITIVE_LIKE: {
    if( zRight ){
      sqlite3RegisterLikeFunctions(db, sqlite3GetBoolean(zRight, 0));
    }
  }
  break;
#endif  
#ifndef SQLITE_INTEGRITY_CHECK_ERROR_MAX
# define SQLITE_INTEGRITY_CHECK_ERROR_MAX 100
#endif
#ifndef SQLITE_OMIT_INTEGRITY_CHECK
  case PragTyp_INTEGRITY_CHECK: {
    int i, j, addr, mxErr;
    int isQuick = (sqlite3Tolower(zLeft[0])=='q');
    assert( iDb>=0 );
    assert( iDb==0 || pId2->z );
    if( pId2->z==0 ) iDb = -1;
    pParse->nMem = 6;
    mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
    if( zRight ){
      sqlite3GetInt32(zRight, &mxErr);
      if( mxErr<=0 ){
        mxErr = SQLITE_INTEGRITY_CHECK_ERROR_MAX;
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, mxErr-1, 1);  
    for(i=0; i<db->nDb; i++){
      HashElem *x;      
      Hash *pTbls;      
      int *aRoot;       
      int cnt = 0;      
      int mxIdx = 0;    
      if( OMIT_TEMPDB && i==1 ) continue;
      if( iDb>=0 && i!=iDb ) continue;
      sqlite3CodeVerifySchema(pParse, i);
      assert( sqlite3SchemaMutexHeld(db, i, 0) );
      pTbls = &db->aDb[i].pSchema->tblHash;
      for(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);   
        Index *pIdx;                       
        int nIdx;                          
        if( HasRowid(pTab) ) cnt++;
        for(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){ cnt++; }
        if( nIdx>mxIdx ) mxIdx = nIdx;
      }
      aRoot = sqlite3DbMallocRawNN(db, sizeof(int)*(cnt+1));
      if( aRoot==0 ) break;
      for(cnt=0, x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx;
        if( HasRowid(pTab) ) aRoot[++cnt] = pTab->tnum;
        for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          aRoot[++cnt] = pIdx->tnum;
        }
      }
      aRoot[0] = cnt;
      pParse->nMem = MAX( pParse->nMem, 8+mxIdx );
      sqlite3ClearTempRegCache(pParse);
      sqlite3VdbeAddOp4(v, OP_IntegrityCk, 2, cnt, 1, (char*)aRoot,P4_INTARRAY);
      sqlite3VdbeChangeP5(v, (u8)i);
      addr = sqlite3VdbeAddOp1(v, OP_IsNull, 2); VdbeCoverage(v);
      sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0,
         sqlite3MPrintf(db, "*** in database %s ***\n", db->aDb[i].zDbSName),
         P4_DYNAMIC);
      sqlite3VdbeAddOp3(v, OP_Concat, 2, 3, 3);
      integrityCheckResultRow(v);
      sqlite3VdbeJumpHere(v, addr);
      for(x=sqliteHashFirst(pTbls); x; x=sqliteHashNext(x)){
        Table *pTab = sqliteHashData(x);
        Index *pIdx, *pPk;
        Index *pPrior = 0;
        int loopTop;
        int iDataCur, iIdxCur;
        int r1 = -1;
        if( pTab->tnum<1 ) continue;   
        pPk = HasRowid(pTab) ? 0 : sqlite3PrimaryKeyIndex(pTab);
        sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenRead, 0,
                                   1, 0, &iDataCur, &iIdxCur);
        sqlite3VdbeAddOp2(v, OP_Integer, 0, 7);
        for(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
          sqlite3VdbeAddOp2(v, OP_Integer, 0, 8+j);  
        }
        assert( pParse->nMem>=8+j );
        assert( sqlite3NoTempsInRange(pParse,1,7+j) );
        sqlite3VdbeAddOp2(v, OP_Rewind, iDataCur, 0); VdbeCoverage(v);
        loopTop = sqlite3VdbeAddOp2(v, OP_AddImm, 7, 1);
        if( !isQuick ){
          sqlite3VdbeAddOp3(v, OP_Column, iDataCur, pTab->nNVCol-1,3);
          sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
        }
        for(j=0; j<pTab->nCol; j++){
          char *zErr;
          int jmp2;
          if( j==pTab->iPKey ) continue;
          if( pTab->aCol[j].notNull==0 ) continue;
          sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, j, 3);
          sqlite3VdbeChangeP5(v, OPFLAG_TYPEOFARG);
          jmp2 = sqlite3VdbeAddOp1(v, OP_NotNull, 3); VdbeCoverage(v);
          zErr = sqlite3MPrintf(db, "NULL value in %s.%s", pTab->zName,
                              pTab->aCol[j].zName);
          sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
          integrityCheckResultRow(v);
          sqlite3VdbeJumpHere(v, jmp2);
        }
        if( pTab->pCheck && (db->flags & SQLITE_IgnoreChecks)==0 ){
          ExprList *pCheck = sqlite3ExprListDup(db, pTab->pCheck, 0);
          if( db->mallocFailed==0 ){
            int addrCkFault = sqlite3VdbeMakeLabel(pParse);
            int addrCkOk = sqlite3VdbeMakeLabel(pParse);
            char *zErr;
            int k;
            pParse->iSelfTab = iDataCur + 1;
            for(k=pCheck->nExpr-1; k>0; k--){
              sqlite3ExprIfFalse(pParse, pCheck->a[k].pExpr, addrCkFault, 0);
            }
            sqlite3ExprIfTrue(pParse, pCheck->a[0].pExpr, addrCkOk, 
                SQLITE_JUMPIFNULL);
            sqlite3VdbeResolveLabel(v, addrCkFault);
            pParse->iSelfTab = 0;
            zErr = sqlite3MPrintf(db, "CHECK constraint failed in %s",
                pTab->zName);
            sqlite3VdbeAddOp4(v, OP_String8, 0, 3, 0, zErr, P4_DYNAMIC);
            integrityCheckResultRow(v);
            sqlite3VdbeResolveLabel(v, addrCkOk);
          }
          sqlite3ExprListDelete(db, pCheck);
        }
        if( !isQuick ){  
          for(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            int jmp2, jmp3, jmp4, jmp5;
            int ckUniq = sqlite3VdbeMakeLabel(pParse);
            if( pPk==pIdx ) continue;
            r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 0, &jmp3,
                                         pPrior, r1);
            pPrior = pIdx;
            sqlite3VdbeAddOp2(v, OP_AddImm, 8+j, 1); 
            jmp2 = sqlite3VdbeAddOp4Int(v, OP_Found, iIdxCur+j, ckUniq, r1,
                                        pIdx->nColumn); VdbeCoverage(v);
            sqlite3VdbeLoadString(v, 3, "row ");
            sqlite3VdbeAddOp3(v, OP_Concat, 7, 3, 3);
            sqlite3VdbeLoadString(v, 4, " missing from index ");
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp5 = sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 3, 3);
            jmp4 = integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, jmp2);
            if( IsUniqueIndex(pIdx) ){
              int uniqOk = sqlite3VdbeMakeLabel(pParse);
              int jmp6;
              int kk;
              for(kk=0; kk<pIdx->nKeyCol; kk++){
                int iCol = pIdx->aiColumn[kk];
                assert( iCol!=XN_ROWID && iCol<pTab->nCol );
                if( iCol>=0 && pTab->aCol[iCol].notNull ) continue;
                sqlite3VdbeAddOp2(v, OP_IsNull, r1+kk, uniqOk);
                VdbeCoverage(v);
              }
              jmp6 = sqlite3VdbeAddOp1(v, OP_Next, iIdxCur+j); VdbeCoverage(v);
              sqlite3VdbeGoto(v, uniqOk);
              sqlite3VdbeJumpHere(v, jmp6);
              sqlite3VdbeAddOp4Int(v, OP_IdxGT, iIdxCur+j, uniqOk, r1,
                                   pIdx->nKeyCol); VdbeCoverage(v);
              sqlite3VdbeLoadString(v, 3, "non-unique entry in index ");
              sqlite3VdbeGoto(v, jmp5);
              sqlite3VdbeResolveLabel(v, uniqOk);
            }
            sqlite3VdbeJumpHere(v, jmp4);
            sqlite3ResolvePartIdxLabel(pParse, jmp3);
          }
        }
        sqlite3VdbeAddOp2(v, OP_Next, iDataCur, loopTop); VdbeCoverage(v);
        sqlite3VdbeJumpHere(v, loopTop-1);
#ifndef SQLITE_OMIT_BTREECOUNT
        if( !isQuick ){
          sqlite3VdbeLoadString(v, 2, "wrong # of entries in index ");
          for(j=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, j++){
            if( pPk==pIdx ) continue;
            sqlite3VdbeAddOp2(v, OP_Count, iIdxCur+j, 3);
            addr = sqlite3VdbeAddOp3(v, OP_Eq, 8+j, 0, 3); VdbeCoverage(v);
            sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
            sqlite3VdbeLoadString(v, 4, pIdx->zName);
            sqlite3VdbeAddOp3(v, OP_Concat, 4, 2, 3);
            integrityCheckResultRow(v);
            sqlite3VdbeJumpHere(v, addr);
          }
        }
#endif  
      } 
    }
    {
      static const int iLn = VDBE_OFFSET_LINENO(2);
      static const VdbeOpList endCode[] = {
        { OP_AddImm,      1, 0,        0},     
        { OP_IfNotZero,   1, 4,        0},     
        { OP_String8,     0, 3,        0},     
        { OP_ResultRow,   3, 1,        0},     
        { OP_Halt,        0, 0,        0},     
        { OP_String8,     0, 3,        0},     
        { OP_Goto,        0, 3,        0},     
      };
      VdbeOp *aOp;
      aOp = sqlite3VdbeAddOpList(v, ArraySize(endCode), endCode, iLn);
      if( aOp ){
        aOp[0].p2 = 1-mxErr;
        aOp[2].p4type = P4_STATIC;
        aOp[2].p4.z = "ok";
        aOp[5].p4type = P4_STATIC;
        aOp[5].p4.z = (char*)sqlite3ErrStr(SQLITE_CORRUPT);
      }
      sqlite3VdbeChangeP3(v, 0, sqlite3VdbeCurrentAddr(v)-2);
    }
  }
  break;
#endif  
#ifndef SQLITE_OMIT_UTF16
  case PragTyp_ENCODING: {
    static const struct EncName {
      char *zName;
      u8 enc;
    } encnames[] = {
      { "UTF8",     SQLITE_UTF8        },
      { "UTF-8",    SQLITE_UTF8        },   
      { "UTF-16le", SQLITE_UTF16LE     },   
      { "UTF-16be", SQLITE_UTF16BE     },   
      { "UTF16le",  SQLITE_UTF16LE     },
      { "UTF16be",  SQLITE_UTF16BE     },
      { "UTF-16",   0                  },  
      { "UTF16",    0                  },  
      { 0, 0 }
    };
    const struct EncName *pEnc;
    if( !zRight ){     
      if( sqlite3ReadSchema(pParse) ) goto pragma_out;
      assert( encnames[SQLITE_UTF8].enc==SQLITE_UTF8 );
      assert( encnames[SQLITE_UTF16LE].enc==SQLITE_UTF16LE );
      assert( encnames[SQLITE_UTF16BE].enc==SQLITE_UTF16BE );
      returnSingleText(v, encnames[ENC(pParse->db)].zName);
    }else{                         
      if( 
        !(DbHasProperty(db, 0, DB_SchemaLoaded)) || 
        DbHasProperty(db, 0, DB_Empty) 
      ){
        for(pEnc=&encnames[0]; pEnc->zName; pEnc++){
          if( 0==sqlite3StrICmp(zRight, pEnc->zName) ){
            SCHEMA_ENC(db) = ENC(db) =
                pEnc->enc ? pEnc->enc : SQLITE_UTF16NATIVE;
            break;
          }
        }
        if( !pEnc->zName ){
          sqlite3ErrorMsg(pParse, "unsupported encoding: %s", zRight);
        }
      }
    }
  }
  break;
#endif  
#ifndef SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS
  case PragTyp_HEADER_VALUE: {
    int iCookie = pPragma->iArg;   
    sqlite3VdbeUsesBtree(v, iDb);
    if( zRight && (pPragma->mPragFlg & PragFlg_ReadOnly)==0 ){
      static const VdbeOpList setCookie[] = {
        { OP_Transaction,    0,  1,  0},     
        { OP_SetCookie,      0,  0,  0},     
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(setCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(setCookie), setCookie, 0);
      if( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p2 = iCookie;
      aOp[1].p3 = sqlite3Atoi(zRight);
    }else{
      static const VdbeOpList readCookie[] = {
        { OP_Transaction,     0,  0,  0},     
        { OP_ReadCookie,      0,  1,  0},     
        { OP_ResultRow,       1,  1,  0}
      };
      VdbeOp *aOp;
      sqlite3VdbeVerifyNoMallocRequired(v, ArraySize(readCookie));
      aOp = sqlite3VdbeAddOpList(v, ArraySize(readCookie),readCookie,0);
      if( ONLY_IF_REALLOC_STRESS(aOp==0) ) break;
      aOp[0].p1 = iDb;
      aOp[1].p1 = iDb;
      aOp[1].p3 = iCookie;
      sqlite3VdbeReusable(v);
    }
  }
  break;
#endif  
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
  case PragTyp_COMPILE_OPTIONS: {
    int i = 0;
    const char *zOpt;
    pParse->nMem = 1;
    while( (zOpt = sqlite3_compileoption_get(i++))!=0 ){
      sqlite3VdbeLoadString(v, 1, zOpt);
      sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 1);
    }
    sqlite3VdbeReusable(v);
  }
  break;
#endif  
#ifndef SQLITE_OMIT_WAL
  case PragTyp_WAL_CHECKPOINT: {
    int iBt = (pId2->z?iDb:SQLITE_MAX_ATTACHED);
    int eMode = SQLITE_CHECKPOINT_PASSIVE;
    if( zRight ){
      if( sqlite3StrICmp(zRight, "full")==0 ){
        eMode = SQLITE_CHECKPOINT_FULL;
      }else if( sqlite3StrICmp(zRight, "restart")==0 ){
        eMode = SQLITE_CHECKPOINT_RESTART;
      }else if( sqlite3StrICmp(zRight, "truncate")==0 ){
        eMode = SQLITE_CHECKPOINT_TRUNCATE;
      }
    }
    pParse->nMem = 3;
    sqlite3VdbeAddOp3(v, OP_Checkpoint, iBt, eMode, 1);
    sqlite3VdbeAddOp2(v, OP_ResultRow, 1, 3);
  }
  break;
  case PragTyp_WAL_AUTOCHECKPOINT: {
    if( zRight ){
      sqlite3_wal_autocheckpoint(db, sqlite3Atoi(zRight));
    }
    returnSingleInt(v, 
       db->xWalCallback==sqlite3WalDefaultHook ? 
           SQLITE_PTR_TO_INT(db->pWalArg) : 0);
  }
  break;
#endif
  case PragTyp_SHRINK_MEMORY: {
    sqlite3_db_release_memory(db);
    break;
  }
  case PragTyp_OPTIMIZE: {
    int iDbLast;            
    int iTabCur;            
    HashElem *k;            
    Schema *pSchema;        
    Table *pTab;            
    Index *pIdx;            
    LogEst szThreshold;     
    char *zSubSql;          
    u32 opMask;             
    if( zRight ){
      opMask = (u32)sqlite3Atoi(zRight);
      if( (opMask & 0x02)==0 ) break;
    }else{
      opMask = 0xfffe;
    }
    iTabCur = pParse->nTab++;
    for(iDbLast = zDb?iDb:db->nDb-1; iDb<=iDbLast; iDb++){
      if( iDb==1 ) continue;
      sqlite3CodeVerifySchema(pParse, iDb);
      pSchema = db->aDb[iDb].pSchema;
      for(k=sqliteHashFirst(&pSchema->tblHash); k; k=sqliteHashNext(k)){
        pTab = (Table*)sqliteHashData(k);
        if( (pTab->tabFlags & TF_StatsUsed)==0 ) continue;
        szThreshold = pTab->nRowLogEst + 46; assert( sqlite3LogEst(25)==46 );
        for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
          if( !pIdx->hasStat1 ){
            szThreshold = 0;  
            break;
          }
        }
        if( szThreshold ){
          sqlite3OpenTable(pParse, iTabCur, iDb, pTab, OP_OpenRead);
          sqlite3VdbeAddOp3(v, OP_IfSmaller, iTabCur, 
                         sqlite3VdbeCurrentAddr(v)+2+(opMask&1), szThreshold);
          VdbeCoverage(v);
        }
        zSubSql = sqlite3MPrintf(db, "ANALYZE \"%w\".\"%w\"",
                                 db->aDb[iDb].zDbSName, pTab->zName);
        if( opMask & 0x01 ){
          int r1 = sqlite3GetTempReg(pParse);
          sqlite3VdbeAddOp4(v, OP_String8, 0, r1, 0, zSubSql, P4_DYNAMIC);
          sqlite3VdbeAddOp2(v, OP_ResultRow, r1, 1);
        }else{
          sqlite3VdbeAddOp4(v, OP_SqlExec, 0, 0, 0, zSubSql, P4_DYNAMIC);
        }
      }
    }
    sqlite3VdbeAddOp0(v, OP_Expire);
    break;
  }
    default: {
    assert( pPragma->ePragTyp==PragTyp_BUSY_TIMEOUT );
    if( zRight ){
      sqlite3_busy_timeout(db, sqlite3Atoi(zRight));
    }
    returnSingleInt(v, db->busyTimeout);
    break;
  }
  case PragTyp_SOFT_HEAP_LIMIT: {
    sqlite3_int64 N;
    if( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_soft_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_soft_heap_limit64(-1));
    break;
  }
  case PragTyp_HARD_HEAP_LIMIT: {
    sqlite3_int64 N;
    if( zRight && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK ){
      sqlite3_int64 iPrior = sqlite3_hard_heap_limit64(-1);
      if( N>0 && (iPrior==0 || iPrior>N) ) sqlite3_hard_heap_limit64(N);
    }
    returnSingleInt(v, sqlite3_hard_heap_limit64(-1));
    break;
  }
  case PragTyp_THREADS: {
    sqlite3_int64 N;
    if( zRight
     && sqlite3DecOrHexToI64(zRight, &N)==SQLITE_OK
     && N>=0
    ){
      sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, (int)(N&0x7fffffff));
    }
    returnSingleInt(v, sqlite3_limit(db, SQLITE_LIMIT_WORKER_THREADS, -1));
    break;
  }
#if defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  case PragTyp_LOCK_STATUS: {
    static const char *const azLockName[] = {
      "unlocked", "shared", "reserved", "pending", "exclusive"
    };
    int i;
    pParse->nMem = 2;
    for(i=0; i<db->nDb; i++){
      Btree *pBt;
      const char *zState = "unknown";
      int j;
      if( db->aDb[i].zDbSName==0 ) continue;
      pBt = db->aDb[i].pBt;
      if( pBt==0 || sqlite3BtreePager(pBt)==0 ){
        zState = "closed";
      }else if( sqlite3_file_control(db, i ? db->aDb[i].zDbSName : 0, 
                                     SQLITE_FCNTL_LOCKSTATE, &j)==SQLITE_OK ){
         zState = azLockName[j];
      }
      sqlite3VdbeMultiLoad(v, 1, "ss", db->aDb[i].zDbSName, zState);
    }
    break;
  }
#endif
#ifdef SQLITE_HAS_CODEC
  case PragTyp_KEY: {
    if( zRight ){
      char zBuf[40];
      const char *zKey = zRight;
      int n;
      if( pPragma->iArg==2 || pPragma->iArg==3 ){
        u8 iByte;
        int i;
        for(i=0, iByte=0; i<sizeof(zBuf)*2 && sqlite3Isxdigit(zRight[i]); i++){
          iByte = (iByte<<4) + sqlite3HexToInt(zRight[i]);
          if( (i&1)!=0 ) zBuf[i/2] = iByte;
        }
        zKey = zBuf;
        n = i/2;
      }else{
        n = pPragma->iArg<4 ? sqlite3Strlen30(zRight) : -1;
      }
      if( (pPragma->iArg & 1)==0 ){
        rc = sqlite3_key_v2(db, zDb, zKey, n);
      }else{
        rc = sqlite3_rekey_v2(db, zDb, zKey, n);
      }
      if( rc==SQLITE_OK && n!=0 ){
        sqlite3VdbeSetNumCols(v, 1);
        sqlite3VdbeSetColName(v, 0, COLNAME_NAME, "ok", SQLITE_STATIC);
        returnSingleText(v, "ok");
      }
    }
    break;
  }
#endif
#if defined(SQLITE_HAS_CODEC) || defined(SQLITE_ENABLE_CEROD)
  case PragTyp_ACTIVATE_EXTENSIONS: if( zRight ){
#ifdef SQLITE_HAS_CODEC
    if( sqlite3StrNICmp(zRight, "see-", 4)==0 ){
      sqlite3_activate_see(&zRight[4]);
    }
#endif
#ifdef SQLITE_ENABLE_CEROD
    if( sqlite3StrNICmp(zRight, "cerod-", 6)==0 ){
      sqlite3_activate_cerod(&zRight[6]);
    }
#endif
  }
  break;
#endif
  }  
  if( (pPragma->mPragFlg & PragFlg_NoColumns1) && zRight ){
    sqlite3VdbeVerifyNoResultRow(v);
  }
pragma_out:
  sqlite3DbFree(db, zLeft);
  sqlite3DbFree(db, zRight);
}