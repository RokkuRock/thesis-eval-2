static int flattenSubquery(
  Parse *pParse,        
  Select *p,            
  int iFrom,            
  int isAgg             
){
  const char *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;     
  Select *pSub;        
  Select *pSub1;       
  SrcList *pSrc;       
  SrcList *pSubSrc;    
  int iParent;         
  int iNewParent = -1; 
  int isLeftJoin = 0;      
  int i;               
  Expr *pWhere;                     
  struct SrcList_item *pSubitem;    
  sqlite3 *db = pParse->db;
  assert( p!=0 );
  assert( p->pPrior==0 );
  if( OptimizationDisabled(db, SQLITE_QueryFlattener) ) return 0;
  pSrc = p->pSrc;
  assert( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;
  pSub = pSubitem->pSelect;
  assert( pSub!=0 );
#ifndef SQLITE_OMIT_WINDOWFUNC
  if( p->pWin || pSub->pWin ) return 0;                   
#endif
  pSubSrc = pSub->pSrc;
  assert( pSubSrc );
  if( pSub->pLimit && p->pLimit ) return 0;               
  if( pSub->pLimit && pSub->pLimit->pRight ) return 0;    
  if( (p->selFlags & SF_Compound)!=0 && pSub->pLimit ){
    return 0;                                             
  }
  if( pSubSrc->nSrc==0 ) return 0;                        
  if( pSub->selFlags & SF_Distinct ) return 0;            
  if( pSub->pLimit && (pSrc->nSrc>1 || isAgg) ){
     return 0;          
  }
  if( p->pOrderBy && pSub->pOrderBy ){
     return 0;                                            
  }
  if( isAgg && pSub->pOrderBy ) return 0;                 
  if( pSub->pLimit && p->pWhere ) return 0;               
  if( pSub->pLimit && (p->selFlags & SF_Distinct)!=0 ){
     return 0;          
  }
  if( pSub->selFlags & (SF_Recursive) ){
    return 0;  
  }
  if( (pSubitem->fg.jointype & JT_OUTER)!=0 ){
    isLeftJoin = 1;
    if( pSubSrc->nSrc>1 || isAgg || IsVirtual(pSubSrc->a[0].pTab) ){
      return 0;
    }
  }
#ifdef SQLITE_EXTRA_IFNULLROW
  else if( iFrom>0 && !isAgg ){
    isLeftJoin = -1;
  }
#endif
  if( pSub->pPrior ){
    if( pSub->pOrderBy ){
      return 0;   
    }
    if( isAgg || (p->selFlags & SF_Distinct)!=0 || pSrc->nSrc!=1 ){
      return 0;  
    }
    for(pSub1=pSub; pSub1; pSub1=pSub1->pPrior){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      assert( pSub->pSrc!=0 );
      assert( pSub->pEList->nExpr==pSub1->pEList->nExpr );
      if( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0     
       || (pSub1->pPrior && pSub1->op!=TK_ALL)                  
       || pSub1->pSrc->nSrc<1                                   
      ){
        return 0;
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }
    if( p->pOrderBy ){
      int ii;
      for(ii=0; ii<p->pOrderBy->nExpr; ii++){
        if( p->pOrderBy->a[ii].u.x.iOrderByCol==0 ) return 0;
      }
    }
  }
  assert( (p->selFlags & SF_Recursive)==0 || pSub->pPrior==0 );
  SELECTTRACE(1,pParse,p,("flatten %u.%p from term %d\n",
                   pSub->selId, pSub, iFrom));
  pParse->zAuthContext = pSubitem->zName;
  TESTONLY(i =) sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0);
  testcase( i==SQLITE_DENY );
  pParse->zAuthContext = zSavedAuthContext;
  for(pSub=pSub->pPrior; pSub; pSub=pSub->pPrior){
    Select *pNew;
    ExprList *pOrderBy = p->pOrderBy;
    Expr *pLimit = p->pLimit;
    Select *pPrior = p->pPrior;
    p->pOrderBy = 0;
    p->pSrc = 0;
    p->pPrior = 0;
    p->pLimit = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->pLimit = pLimit;
    p->pOrderBy = pOrderBy;
    p->pSrc = pSrc;
    p->op = TK_ALL;
    if( pNew==0 ){
      p->pPrior = pPrior;
    }else{
      pNew->pPrior = pPrior;
      if( pPrior ) pPrior->pNext = pNew;
      pNew->pNext = p;
      p->pPrior = pNew;
      SELECTTRACE(2,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    if( db->mallocFailed ) return 1;
  }
  pSub = pSub1 = pSubitem->pSelect;
  sqlite3DbFree(db, pSubitem->zDatabase);
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zDatabase = 0;
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  pSubitem->pSelect = 0;
  if( ALWAYS(pSubitem->pTab!=0) ){
    Table *pTabToDel = pSubitem->pTab;
    if( pTabToDel->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      pTabToDel->pNextZombie = pToplevel->pZombieTab;
      pToplevel->pZombieTab = pTabToDel;
    }else{
      pTabToDel->nTabRef--;
    }
    pSubitem->pTab = 0;
  }
  for(pParent=p; pParent; pParent=pParent->pPrior, pSub=pSub->pPrior){
    int nSubSrc;
    u8 jointype = 0;
    assert( pSub!=0 );
    pSubSrc = pSub->pSrc;      
    nSubSrc = pSubSrc->nSrc;   
    pSrc = pParent->pSrc;      
    if( pSrc ){
      assert( pParent==p );   
      jointype = pSubitem->fg.jointype;
    }else{
      assert( pParent!=p );   
      pSrc = sqlite3SrcListAppend(pParse, 0, 0, 0);
      if( pSrc==0 ) break;
      pParent->pSrc = pSrc;
    }
    if( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      if( pSrc==0 ) break;
      pParent->pSrc = pSrc;
    }
    for(i=0; i<nSubSrc; i++){
      sqlite3IdListDelete(db, pSrc->a[i+iFrom].pUsing);
      assert( pSrc->a[i+iFrom].fg.isTabFunc==0 );
      pSrc->a[i+iFrom] = pSubSrc->a[i];
      iNewParent = pSubSrc->a[i].iCursor;
      memset(&pSubSrc->a[i], 0, sizeof(pSubSrc->a[i]));
    }
    pSrc->a[iFrom].fg.jointype = jointype;
    if( pSub->pOrderBy ){
      ExprList *pOrderBy = pSub->pOrderBy;
      for(i=0; i<pOrderBy->nExpr; i++){
        pOrderBy->a[i].u.x.iOrderByCol = 0;
      }
      assert( pParent->pOrderBy==0 );
      pParent->pOrderBy = pOrderBy;
      pSub->pOrderBy = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    if( isLeftJoin>0 ){
      sqlite3SetJoinExpr(pWhere, iNewParent);
    }
    pParent->pWhere = sqlite3ExprAnd(pParse, pWhere, pParent->pWhere);
    if( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.iTable = iParent;
      x.iNewTable = iNewParent;
      x.isLeftJoin = isLeftJoin;
      x.pEList = pSub->pEList;
      substSelect(&x, pParent, 0);
    }
    pParent->selFlags |= pSub->selFlags & SF_Compound;
    assert( (pSub->selFlags & SF_Distinct)==0 );  
    if( pSub->pLimit ){
      pParent->pLimit = pSub->pLimit;
      pSub->pLimit = 0;
    }
  }
  sqlite3SelectDelete(db, pSub1);
#if SELECTTRACE_ENABLED
  if( sqlite3SelectTrace & 0x100 ){
    SELECTTRACE(0x100,pParse,p,("After flattening:\n"));
    sqlite3TreeViewSelect(0, p, 0);
  }
#endif
  return 1;
}