int sqlite3WindowRewrite(Parse *pParse, Select *p){
  int rc = SQLITE_OK;
  if( p->pWin && p->pPrior==0 && (p->selFlags & SF_WinRewrite)==0 ){
    Vdbe *v = sqlite3GetVdbe(pParse);
    sqlite3 *db = pParse->db;
    Select *pSub = 0;              
    SrcList *pSrc = p->pSrc;
    Expr *pWhere = p->pWhere;
    ExprList *pGroupBy = p->pGroupBy;
    Expr *pHaving = p->pHaving;
    ExprList *pSort = 0;
    ExprList *pSublist = 0;        
    Window *pMWin = p->pWin;       
    Window *pWin;                  
    Table *pTab;
    pTab = sqlite3DbMallocZero(db, sizeof(Table));
    if( pTab==0 ){
      return SQLITE_NOMEM;
    }
    p->pSrc = 0;
    p->pWhere = 0;
    p->pGroupBy = 0;
    p->pHaving = 0;
    p->selFlags &= ~SF_Aggregate;
    p->selFlags |= SF_WinRewrite;
    pSort = sqlite3ExprListDup(db, pMWin->pPartition, 0);
    pSort = exprListAppendList(pParse, pSort, pMWin->pOrderBy, 1);
    if( pSort && p->pOrderBy && p->pOrderBy->nExpr<=pSort->nExpr ){
      int nSave = pSort->nExpr;
      pSort->nExpr = p->pOrderBy->nExpr;
      if( sqlite3ExprListCompare(pSort, p->pOrderBy, -1)==0 ){
        sqlite3ExprListDelete(db, p->pOrderBy);
        p->pOrderBy = 0;
      }
      pSort->nExpr = nSave;
    }
    pMWin->iEphCsr = pParse->nTab++;
    pParse->nTab += 3;
    selectWindowRewriteEList(pParse, pMWin, pSrc, p->pEList, pTab, &pSublist);
    selectWindowRewriteEList(pParse, pMWin, pSrc, p->pOrderBy, pTab, &pSublist);
    pMWin->nBufferCol = (pSublist ? pSublist->nExpr : 0);
    pSublist = exprListAppendList(pParse, pSublist, pMWin->pPartition, 0);
    pSublist = exprListAppendList(pParse, pSublist, pMWin->pOrderBy, 0);
    for(pWin=pMWin; pWin; pWin=pWin->pNextWin){
      ExprList *pArgs = pWin->pOwner->x.pList;
      if( pWin->pFunc->funcFlags & SQLITE_FUNC_SUBTYPE ){
        selectWindowRewriteEList(pParse, pMWin, pSrc, pArgs, pTab, &pSublist);
        pWin->iArgCol = (pSublist ? pSublist->nExpr : 0);
        pWin->bExprArgs = 1;
      }else{
        pWin->iArgCol = (pSublist ? pSublist->nExpr : 0);
        pSublist = exprListAppendList(pParse, pSublist, pArgs, 0);
      }
      if( pWin->pFilter ){
        Expr *pFilter = sqlite3ExprDup(db, pWin->pFilter, 0);
        pSublist = sqlite3ExprListAppend(pParse, pSublist, pFilter);
      }
      pWin->regAccum = ++pParse->nMem;
      pWin->regResult = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, pWin->regAccum);
    }
    if( pSublist==0 ){
      pSublist = sqlite3ExprListAppend(pParse, 0, 
        sqlite3Expr(db, TK_INTEGER, "0")
      );
    }
    pSub = sqlite3SelectNew(
        pParse, pSublist, pSrc, pWhere, pGroupBy, pHaving, pSort, 0, 0
    );
    p->pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
    if( p->pSrc ){
      Table *pTab2;
      p->pSrc->a[0].pSelect = pSub;
      sqlite3SrcListAssignCursors(pParse, p->pSrc);
      pSub->selFlags |= SF_Expanded;
      pTab2 = sqlite3ResultSetOfSelect(pParse, pSub, SQLITE_AFF_NONE);
      if( pTab2==0 ){
        rc = SQLITE_NOMEM;
      }else{
        memcpy(pTab, pTab2, sizeof(Table));
        pTab->tabFlags |= TF_Ephemeral;
        p->pSrc->a[0].pTab = pTab;
        pTab = pTab2;
      }
      sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pMWin->iEphCsr, pSublist->nExpr);
      sqlite3VdbeAddOp2(v, OP_OpenDup, pMWin->iEphCsr+1, pMWin->iEphCsr);
      sqlite3VdbeAddOp2(v, OP_OpenDup, pMWin->iEphCsr+2, pMWin->iEphCsr);
      sqlite3VdbeAddOp2(v, OP_OpenDup, pMWin->iEphCsr+3, pMWin->iEphCsr);
    }else{
      sqlite3SelectDelete(db, pSub);
    }
    if( db->mallocFailed ) rc = SQLITE_NOMEM;
    sqlite3DbFree(db, pTab);
  }
  return rc;
}