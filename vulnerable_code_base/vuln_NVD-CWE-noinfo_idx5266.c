void sqlite3EndTable(
  Parse *pParse,           
  Token *pCons,            
  Token *pEnd,             
  u8 tabOpts,              
  Select *pSelect          
){
  Table *p;                  
  sqlite3 *db = pParse->db;  
  int iDb;                   
  Index *pIdx;               
  if( pEnd==0 && pSelect==0 ){
    return;
  }
  assert( !db->mallocFailed );
  p = pParse->pNewTable;
  if( p==0 ) return;
  if( pSelect==0 && isShadowTableName(db, p->zName) ){
    p->tabFlags |= TF_Shadow;
  }
  if( db->init.busy ){
    if( pSelect ){
      sqlite3ErrorMsg(pParse, "");
      return;
    }
    p->tnum = db->init.newTnum;
    if( p->tnum==1 ) p->tabFlags |= TF_Readonly;
  }
  assert( (p->tabFlags & TF_HasPrimaryKey)==0
       || p->iPKey>=0 || sqlite3PrimaryKeyIndex(p)!=0 );
  assert( (p->tabFlags & TF_HasPrimaryKey)!=0
       || (p->iPKey<0 && sqlite3PrimaryKeyIndex(p)==0) );
  if( tabOpts & TF_WithoutRowid ){
    if( (p->tabFlags & TF_Autoincrement) ){
      sqlite3ErrorMsg(pParse,
          "AUTOINCREMENT not allowed on WITHOUT ROWID tables");
      return;
    }
    if( (p->tabFlags & TF_HasPrimaryKey)==0 ){
      sqlite3ErrorMsg(pParse, "PRIMARY KEY missing on table %s", p->zName);
      return;
    }
    p->tabFlags |= TF_WithoutRowid | TF_NoVisibleRowid;
    convertToWithoutRowidTable(pParse, p);
  }
  iDb = sqlite3SchemaToIndex(db, p->pSchema);
#ifndef SQLITE_OMIT_CHECK
  if( p->pCheck ){
    sqlite3ResolveSelfReference(pParse, p, NC_IsCheck, 0, p->pCheck);
  }
#endif  
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  if( p->tabFlags & TF_HasGenerated ){
    int ii, nNG = 0;
    testcase( p->tabFlags & TF_HasVirtual );
    testcase( p->tabFlags & TF_HasStored );
    for(ii=0; ii<p->nCol; ii++){
      u32 colFlags = p->aCol[ii].colFlags;
      if( (colFlags & COLFLAG_GENERATED)!=0 ){
        testcase( colFlags & COLFLAG_VIRTUAL );
        testcase( colFlags & COLFLAG_STORED );
        sqlite3ResolveSelfReference(pParse, p, NC_GenCol, 
                                    p->aCol[ii].pDflt, 0);
      }else{
        nNG++;
      }
    }
    if( nNG==0 ){
      sqlite3ErrorMsg(pParse, "must have at least one non-generated column");
      return;
    }
  }
#endif
  estimateTableWidth(p);
  for(pIdx=p->pIndex; pIdx; pIdx=pIdx->pNext){
    estimateIndexWidth(pIdx);
  }
  if( !db->init.busy ){
    int n;
    Vdbe *v;
    char *zType;     
    char *zType2;    
    char *zStmt;     
    v = sqlite3GetVdbe(pParse);
    if( NEVER(v==0) ) return;
    sqlite3VdbeAddOp1(v, OP_Close, 0);
    if( p->pSelect==0 ){
      zType = "table";
      zType2 = "TABLE";
#ifndef SQLITE_OMIT_VIEW
    }else{
      zType = "view";
      zType2 = "VIEW";
#endif
    }
    if( pSelect ){
      SelectDest dest;     
      int regYield;        
      int addrTop;         
      int regRec;          
      int regRowid;        
      int addrInsLoop;     
      Table *pSelTab;      
      regYield = ++pParse->nMem;
      regRec = ++pParse->nMem;
      regRowid = ++pParse->nMem;
      assert(pParse->nTab==1);
      sqlite3MayAbort(pParse);
      sqlite3VdbeAddOp3(v, OP_OpenWrite, 1, pParse->regRoot, iDb);
      sqlite3VdbeChangeP5(v, OPFLAG_P2ISREG);
      pParse->nTab = 2;
      addrTop = sqlite3VdbeCurrentAddr(v) + 1;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, addrTop);
      if( pParse->nErr ) return;
      pSelTab = sqlite3ResultSetOfSelect(pParse, pSelect, SQLITE_AFF_BLOB);
      if( pSelTab==0 ) return;
      assert( p->aCol==0 );
      p->nCol = p->nNVCol = pSelTab->nCol;
      p->aCol = pSelTab->aCol;
      pSelTab->nCol = 0;
      pSelTab->aCol = 0;
      sqlite3DeleteTable(db, pSelTab);
      sqlite3SelectDestInit(&dest, SRT_Coroutine, regYield);
      sqlite3Select(pParse, pSelect, &dest);
      if( pParse->nErr ) return;
      sqlite3VdbeEndCoroutine(v, regYield);
      sqlite3VdbeJumpHere(v, addrTop - 1);
      addrInsLoop = sqlite3VdbeAddOp1(v, OP_Yield, dest.iSDParm);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, dest.iSdst, dest.nSdst, regRec);
      sqlite3TableAffinity(v, p, 0);
      sqlite3VdbeAddOp2(v, OP_NewRowid, 1, regRowid);
      sqlite3VdbeAddOp3(v, OP_Insert, 1, regRec, regRowid);
      sqlite3VdbeGoto(v, addrInsLoop);
      sqlite3VdbeJumpHere(v, addrInsLoop);
      sqlite3VdbeAddOp1(v, OP_Close, 1);
    }
    if( pSelect ){
      zStmt = createTableStmt(db, p);
    }else{
      Token *pEnd2 = tabOpts ? &pParse->sLastToken : pEnd;
      n = (int)(pEnd2->z - pParse->sNameToken.z);
      if( pEnd2->z[0]!=';' ) n += pEnd2->n;
      zStmt = sqlite3MPrintf(db, 
          "CREATE %s %.*s", zType2, n, pParse->sNameToken.z
      );
    }
    sqlite3NestedParse(pParse,
      "UPDATE %Q.%s "
         "SET type='%s', name=%Q, tbl_name=%Q, rootpage=#%d, sql=%Q "
       "WHERE rowid=#%d",
      db->aDb[iDb].zDbSName, MASTER_NAME,
      zType,
      p->zName,
      p->zName,
      pParse->regRoot,
      zStmt,
      pParse->regRowid
    );
    sqlite3DbFree(db, zStmt);
    sqlite3ChangeCookie(pParse, iDb);
#ifndef SQLITE_OMIT_AUTOINCREMENT
    if( (p->tabFlags & TF_Autoincrement)!=0 ){
      Db *pDb = &db->aDb[iDb];
      assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
      if( pDb->pSchema->pSeqTab==0 ){
        sqlite3NestedParse(pParse,
          "CREATE TABLE %Q.sqlite_sequence(name,seq)",
          pDb->zDbSName
        );
      }
    }
#endif
    sqlite3VdbeAddParseSchemaOp(v, iDb,
           sqlite3MPrintf(db, "tbl_name='%q' AND type!='trigger'", p->zName));
  }
  if( db->init.busy ){
    Table *pOld;
    Schema *pSchema = p->pSchema;
    assert( sqlite3SchemaMutexHeld(db, iDb, 0) );
    pOld = sqlite3HashInsert(&pSchema->tblHash, p->zName, p);
    if( pOld ){
      assert( p==pOld );   
      sqlite3OomFault(db);
      return;
    }
    pParse->pNewTable = 0;
    db->mDbFlags |= DBFLAG_SchemaChange;
#ifndef SQLITE_OMIT_ALTERTABLE
    if( !p->pSelect ){
      const char *zName = (const char *)pParse->sNameToken.z;
      int nName;
      assert( !pSelect && pCons && pEnd );
      if( pCons->z==0 ){
        pCons = pEnd;
      }
      nName = (int)((const char *)pCons->z - zName);
      p->addColOffset = 13 + sqlite3Utf8CharLen(zName, nName);
    }
#endif
  }
}